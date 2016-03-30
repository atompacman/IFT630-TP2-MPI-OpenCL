#include "MPIUtils.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <mpi.h>
#include <cmath>

uint32_t    const NUM_ROWS = 4;
MPI_Request const ON_VACATION = std::numeric_limits<MPI_Request>().max();

static void printSquareMatrix(double * i_Matrix, char const * i_Name)
{
	std::cout << i_Name << ":" << std::endl;
	for (auto y = 0; y < NUM_ROWS; ++y)
	{
		for (auto x = 0; x < NUM_ROWS; ++x)
		{
			std::cout << std::setw(6) << i_Matrix[y * NUM_ROWS + x] << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

double computeSquaredNormDifference(double *oldMatrix, double *newMatrix)
{
	double squaredNorm = 0.0;
	for (auto y = 1U; y < NUM_ROWS - 1; ++y)
	{
		for (auto x = 1U; x < NUM_ROWS - 1; ++x)
		{
			squaredNorm += (newMatrix[y * NUM_ROWS + x] - oldMatrix[y * NUM_ROWS + x]) * (newMatrix[y * NUM_ROWS + x] - oldMatrix[y * NUM_ROWS + x]);
		}

	}
	return squaredNorm;
}

void sendJob(double      * i_Matrix, 
			 double      * i_Results,
             MPI_Request * i_PendingRequests,
             uint32_t      i_rowNumber,
             uint32_t      i_ProcessID)
{
    // Send row buffers to worker
    MPI_Send(i_Matrix + (i_rowNumber - 1) * NUM_ROWS, 3 * NUM_ROWS, MPI_DOUBLE, i_ProcessID, 0, MPI_COMM_WORLD);

    // Post non-blocking receive to be ready for result reception
    MPI_Request request;
    MPI_Irecv(i_Results + 1 + i_rowNumber * NUM_ROWS, NUM_ROWS - 2, MPI_DOUBLE, i_ProcessID, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
    i_PendingRequests[i_ProcessID - 1] = request;
}

void sendOnVacation(uint32_t i_ProcessID, MPI_Request * i_PendingRequests)
{
    double dummy;
    MPI_Send(&dummy, 1, MPI_DOUBLE, i_ProcessID, ON_VACATION, MPI_COMM_WORLD);
    i_PendingRequests[i_ProcessID - 1] = ON_VACATION;
}

void runManager(uint32_t, uint32_t i_NumProc)
{
	// Create matrix
	double matrix[NUM_ROWS * NUM_ROWS];
	double oldMatrix[NUM_ROWS * NUM_ROWS];

	for (auto y = 0U; y < NUM_ROWS; ++y)
	{
		matrix[y * NUM_ROWS] = -1.0;
		matrix[y * NUM_ROWS + NUM_ROWS - 1] = -1.0;
		auto rowValue = y == 0 || y == NUM_ROWS - 1 ? -1.0 : y;
		for (auto x = 1U; x < NUM_ROWS - 1; ++x)
		{
			matrix[y * NUM_ROWS + x] = rowValue;
		}
	}

	// Print matrix
	printSquareMatrix(matrix, "Initial");

	double difference;
	int iter = 0;

	auto numWorkers = std::min(i_NumProc - 1, NUM_ROWS - 2);
	auto pendingRequests = new MPI_Request[i_NumProc];

	// Send useless workers to vacation
	for (auto i = NUM_ROWS - 1; i < i_NumProc; ++i)
	{
		sendOnVacation(i, pendingRequests);
	}

	do
	{
		++iter;
		std::cout << "Now starting iteration #" << iter << std::endl;
		difference = 0.0;
		// "Save" the current matrix before it gets changed by the computation in order to compare the norm difference to an epsilon
		memcpy(oldMatrix, matrix, NUM_ROWS * NUM_ROWS * sizeof(double));

		std::cout << "Matrix copied. About to distribute the jacobi computation... " << std::endl;

		// Send a first job to each worker
		for (auto i = 0U; i < numWorkers; ++i)
		{
			sendJob(oldMatrix, matrix, pendingRequests, i + 1, i + 1);
		}

		// Gather results and give remaining jobs to available processes
		auto jobsSent = numWorkers;
		auto jobsDone = 0U;
		while (jobsDone < NUM_ROWS - 2)
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

				if (jobsSent < NUM_ROWS - 2)
				{
					// Here's more work !
					sendJob(oldMatrix, matrix, pendingRequests, jobsSent + 1, i + 1);
					++jobsSent;
				}
			}
		}

		std::cout << "Results received. About to compute norm difference... " << std::endl;
		
		// Compute norm difference between matrices
		difference = sqrt(computeSquaredNormDifference(oldMatrix, matrix));
		
		// Print information
		std::cout << "Iteration #" << iter << std::endl;
		std::cout << "Difference: " << difference << std::endl;
		printSquareMatrix(matrix, "new matrix");
		// printSquareMatrix(oldMatrix, NUM_ROWS, "old matrix");

	} while (difference > 1.0e-2 && iter < 100);

	for (auto i = 0U; i < numWorkers; ++i)
	{
		sendOnVacation(i + 1, pendingRequests);
	}

    delete[] pendingRequests;
}

void runWorker(uint32_t, uint32_t)
{
	// Get matrix rows to process
	double input[3 * NUM_ROWS];

	while (true)
	{
		MPI_Status status;
		MPI_Recv(input, 3 * NUM_ROWS, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

		// Check if there is no job left to do
		if (status.MPI_TAG == ON_VACATION)
		{
			return;
		}

		// Computation
		double result[NUM_ROWS - 2];
		for (auto i = 1U; i < NUM_ROWS - 1; ++i)
		{
			result[i - 1] = (input[i] +
				input[NUM_ROWS + i + 1] +
				input[2 * NUM_ROWS + i] +
				input[NUM_ROWS + i - 1]) / 4.0;
		}

		// Send result back
		MPI_Request dummy;
		MPI_Isend(result, NUM_ROWS - 2, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &dummy);

	}
}

int main(int argc, char ** argv)
{
    runManagerWorkerAlgorithm(argc, argv, runManager, runWorker);
}