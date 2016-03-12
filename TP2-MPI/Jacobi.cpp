#include "MPIUtils.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <mpi.h>

uint32_t    const MATRIX_SIZE = 20;
uint32_t    const NUM_ELEM    = MATRIX_SIZE * MATRIX_SIZE;
MPI_Request const ON_VACATION = std::numeric_limits<MPI_Request>().max();

void printMatrix(double * i_Matrix)
{
    static uint32_t k = 0;

    std::cout << k << ":" << std::endl;
    for (auto y = 0; y < MATRIX_SIZE; ++y)
    {
        for (auto x = 0; x < MATRIX_SIZE; ++x)
        {
            std::cout << std::setw(6) << i_Matrix[y * MATRIX_SIZE + x] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    ++k;
}

void sendJob(double      * i_InMatrix,
             double      * i_OutMatrix,
             double      * i_RowsBuf,
             MPI_Request * i_PendingRequests,
             uint32_t      i_ProcessID)
{
    // Copy matrix rows in the row buffer
    for (auto i = 0U; i < 3; ++i)
    {
        memcpy(i_RowsBuf + i * MATRIX_SIZE, 
               i_InMatrix + (i_ProcessID + i - 1) * MATRIX_SIZE, 
               MATRIX_SIZE * sizeof(double));
    }

    // Send row buffers to worker
    MPI_Send(i_RowsBuf, 3 * MATRIX_SIZE, MPI_DOUBLE, i_ProcessID, 0, MPI_COMM_WORLD);

    // Post non-blocking receive to be ready for result reception
    MPI_Request request;
    MPI_Irecv(i_OutMatrix + i_ProcessID * MATRIX_SIZE + 1, MATRIX_SIZE - 2, 
              MPI_DOUBLE, i_ProcessID, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
    i_PendingRequests[i_ProcessID - 1] = request;
}

void sendOnVacation(uint32_t i_ProcessID, MPI_Request * i_PendingRequests)
{
    double dummy[3 * MATRIX_SIZE];
    MPI_Send(dummy, 3 * MATRIX_SIZE, MPI_DOUBLE, i_ProcessID, ON_VACATION, MPI_COMM_WORLD);
    i_PendingRequests[i_ProcessID - 1] = ON_VACATION;
}

void tellWorkerTheresNoJobForHim(uint32_t i_ProcessID)
{
    double dummy[3 * MATRIX_SIZE];
    MPI_Send(dummy, 3 * MATRIX_SIZE, MPI_DOUBLE, i_ProcessID, ON_VACATION, MPI_COMM_WORLD);
}

void runManager(uint32_t, uint32_t i_NumProc)
{
    // Create initial matrices
    double matrices[2][NUM_ELEM];
    for (auto i = 0U; i < 2; ++i)
    {
        auto * matrix = matrices[i];
        for (auto y = 0U; y < MATRIX_SIZE; ++y)
        {
            matrix[y * MATRIX_SIZE] = -1.0;
            matrix[y * MATRIX_SIZE + MATRIX_SIZE - 1] = -1.0;
            auto rowValue = y == 0 || y == MATRIX_SIZE - 1 ? -1.0 : y;
            for (auto x = 1U; x < MATRIX_SIZE - 1; ++x)
            {
                matrix[y * MATRIX_SIZE + x] = rowValue;
            }
        }
    }

    // Print input matrices
    printMatrix(matrices[0]);
    printMatrix(matrices[1]);

    // Send a first job to each worker
    bool   currMatrix = 0;
    double rowsBuf[3 * MATRIX_SIZE];
    auto   numWorkers = std::min(i_NumProc - 1, MATRIX_SIZE - 2);
    auto   pendingRequests = new MPI_Request[numWorkers];
    for (auto i = 0U; i < numWorkers; ++i)
    {
        sendJob(matrices[currMatrix], matrices[!currMatrix], rowsBuf, pendingRequests, i + 1);
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
        // Get 3 matrix rows for computation
        double buf[3 * MATRIX_SIZE];
        MPI_Status status;
        MPI_Recv(buf, 2 * MATRIX_SIZE, MPI_DOUBLE, MANAGER_ID,
                 MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // Check if there is no job left to do
        if (status.MPI_TAG == ON_VACATION)
        {
            return;
        }

        // Actual computation
        double results[MATRIX_SIZE - 2];
        for (auto i = 1U; i < MATRIX_SIZE - 1; ++i)
        {
            results[i - 1] = (buf[i] +                         // UP
                              buf[MATRIX_SIZE + i + 1] +       // RIGHT
                              buf[2 * MATRIX_SIZE + i] +       // DOWN
                              buf[MATRIX_SIZE + i - 1]) / 4.0; // LEFT
        }

        // Send result back
        MPI_Request dummy;
        MPI_Isend(&results, MATRIX_SIZE - 2, MPI_DOUBLE, MANAGER_ID, 0, MPI_COMM_WORLD, &dummy);
    }
}

int main(int argc, char ** argv)
{
    runManagerWorkerAlgorithm(argc, argv, runManager, runWorker);
}