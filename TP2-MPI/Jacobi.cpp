#include "MPIUtils.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <mpi.h>

uint32_t    const MATRIX_SIZE = 20;
uint32_t    const NUM_ELEM = MATRIX_SIZE * MATRIX_SIZE;
MPI_Request const ON_VACATION = std::numeric_limits<MPI_Request>().max();

void printMatrix(double * i_Matrix, char const * i_Name, bool i_DoTranspose = false)
{
    std::cout << i_Name << ":" << std::endl;
    for (auto y = 0; y < MATRIX_SIZE; ++y)
    {
        for (auto x = 0; x < MATRIX_SIZE; ++x)
        {
            auto val = i_Matrix[i_DoTranspose ? x * MATRIX_SIZE + y : y * MATRIX_SIZE + x];
            std::cout << std::setw(6) << val << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void sendJob(double      * i_MatrixA,
             double      * i_MatrixBt,
             double      * i_Results,
             double      * i_RowsBuf,
             MPI_Request * i_PendingRequests,
             uint32_t      i_MatrixComponent,
             uint32_t      i_ProcessID)
{
    // Compute position in matrices where concerned rows are
    auto row = i_MatrixComponent / MATRIX_SIZE;
    auto col = i_MatrixComponent % MATRIX_SIZE;

    // Copy matrix rows in the row buffer
    memcpy(i_RowsBuf, i_MatrixA + row * MATRIX_SIZE, MATRIX_SIZE * sizeof(double));
    memcpy(i_RowsBuf + MATRIX_SIZE, i_MatrixBt + col * MATRIX_SIZE, MATRIX_SIZE * sizeof(double));

    // Send row buffers to worker
    MPI_Send(i_RowsBuf, 2 * MATRIX_SIZE, MPI_DOUBLE, i_ProcessID, 0, MPI_COMM_WORLD);

    // Post non-blocking receive to be ready for result reception
    MPI_Request request;
    auto *      result = i_Results + i_MatrixComponent;
    MPI_Irecv(result, 1, MPI_DOUBLE, i_ProcessID, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
    i_PendingRequests[i_ProcessID - 1] = request;
}

void sendOnVacation(uint32_t i_ProcessID, MPI_Request * i_PendingRequests)
{
    double dummy[2 * MATRIX_SIZE];
    MPI_Send(dummy, 2 * MATRIX_SIZE, MPI_DOUBLE, i_ProcessID, ON_VACATION, MPI_COMM_WORLD);
    i_PendingRequests[i_ProcessID - 1] = ON_VACATION;
}

void runManager(uint32_t, uint32_t i_NumProc)
{
    // Create matrices
    double matrixA[NUM_ELEM];
    double matrixBt[NUM_ELEM];
    double results[NUM_ELEM];
    for (auto i = 0U; i < NUM_ELEM; ++i)
    {
        // Fill with arbitrary values
        matrixA[i] = static_cast<double>(i);
        matrixBt[i] = -static_cast<double>(i) + NUM_ELEM;
    }

    // Print input matrices
    printMatrix(matrixA, "A");
    printMatrix(matrixBt, "B", true);

    // Send a first job to each worker
    double rowsBuf[2 * MATRIX_SIZE];
    auto   numWorkers = std::min(i_NumProc - 1, NUM_ELEM);
    auto   pendingRequests = new MPI_Request[i_NumProc];
    for (auto i = 0U; i < numWorkers; ++i)
    {
        sendJob(matrixA, matrixBt, results, rowsBuf, pendingRequests, i, i + 1);
    }

    // Send useless workers to vacation
    for (auto i = NUM_ELEM + 1; i < i_NumProc; ++i)
    {
        sendOnVacation(i, pendingRequests);
    }

    // Gather results and give remaining jobs to available processes
    auto jobsSent = numWorkers;
    auto jobsDone = 0U;
    while (jobsDone < NUM_ELEM)
    {
        // Check each worker for valid results
        for (auto i = 0U; i < numWorkers; ++i)
        {
            // Are you already on vacation ?
            auto request = pendingRequests[i];
            if (request == ON_VACATION)
            {
                continue;
            }

            // No ? Well, let's see if your work is done...
            auto isJobDone = 0;
            MPI_Test(&request, &isJobDone, MPI_STATUS_IGNORE);

            // Oh, I see, you're still working...
            if (!isJobDone)
            {
                continue;
            }

            // Good job, man!
            ++jobsDone;

            if (jobsSent == NUM_ELEM)
            {
                // I ain't got no more work for you to do
                sendOnVacation(i + 1, pendingRequests);
            }
            else
            {
                // Here's more work !
                sendJob(matrixA, matrixBt, results, rowsBuf, pendingRequests, jobsSent, i + 1);
                ++jobsSent;
            }
        }
    }

    // Print results
    printMatrix(results, "AB", false, true);

    delete[] pendingRequests;
}

void runWorker(uint32_t, uint32_t)
{
    while (true)
    {
        // Get matrix rows for dot product computation
        double buf[2 * MATRIX_SIZE];
        MPI_Status status;
        MPI_Recv(buf, 2 * MATRIX_SIZE, MPI_DOUBLE, MANAGER_ID,
            MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // Check if there is no job left to do
        if (status.MPI_TAG == ON_VACATION)
        {
            return;
        }

        // Dot product
        auto result = 0.0;
        for (auto i = 0U; i < MATRIX_SIZE; ++i)
        {
            result += buf[i] * buf[i + MATRIX_SIZE];
        }

        // Send result back
        MPI_Request dummy;
        MPI_Isend(&result, 1, MPI_DOUBLE, MANAGER_ID, 0, MPI_COMM_WORLD, &dummy);
    }
}

int main(int argc, char ** argv)
{
    runManagerWorkerAlgorithm(argc, argv, runManager, runWorker);
}