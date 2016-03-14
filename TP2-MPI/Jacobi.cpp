#include "MPIUtils.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <mpi.h>
#include <cmath>

static void printSquareMatrix(double * i_Matrix, int i_NumRows, char const * i_Name)
{
	std::cout << i_Name << ":" << std::endl;
	for (auto y = 0; y < i_NumRows; ++y)
	{
		for (auto x = 0; x < i_NumRows; ++x)
		{
			std::cout << std::setw(6) << i_Matrix[y * i_NumRows + x] << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}


double computeSquaredNormDifference(double *oldMatrix, double *newMatrix, int size) 
{
	double squaredNorm = 0.0;
	for (auto y = 1; y < size - 1; ++y)
	{
		for (auto x = 1; x < size - 1; ++x)
		{
			squaredNorm += (newMatrix[y * size + x] - oldMatrix[y * size + x]) * (newMatrix[y * size + x] - oldMatrix[y * size + x]);
		}
		
	}
	return squaredNorm;
}

void runManager(uint32_t, uint32_t i_NumProc)
{
	int numProcesses;
	MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);

	// Create matrix
	auto numRows = numProcesses + 1;
	auto * matrix = new double[numRows * numRows];
	auto * oldMatrix = new double[numRows * numRows];

	for (auto y = 0U; y < numRows; ++y)
	{
		matrix[y * numRows] = -1.0;
		matrix[y * numRows + numRows - 1] = -1.0;
		auto rowValue = y == 0 || y == numRows - 1 ? -1.0 : y;
		for (auto x = 1U; x < numRows - 1; ++x)
		{
			matrix[y * numRows + x] = rowValue;
		}
	}

	// Print matrix
	printSquareMatrix(matrix, numRows, "Initial");
	double difference;
	int iter = 0;
	do
	{
		++iter;
		std::cout << "Now starting iteration #" << iter << std::endl;
		difference = 0.0;
		// "Save" the current matrix before it gets changed by the computation in order to compare the norm difference to an epsilon
		memcpy(oldMatrix, matrix, numRows * numRows * sizeof(double));

		std::cout << "Matrix copied. About to distribute the jacobi computation... " << std::endl;

		// Send rows to workers
		for (auto y = 0U; y < numRows - 2; ++y)
		{
			MPI_Send(matrix + y * numRows, 3 * numRows, MPI_DOUBLE, y+1, 0, MPI_COMM_WORLD);
		}

		std::cout << "Work sent. About to receive... " << std::endl;

		// Receive the resulting rows from the workers
		for (auto y = 1U; y < numRows - 1; ++y)
		{
			MPI_Recv(matrix + 1 + y * numRows, numRows - 2, MPI_DOUBLE, y, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}

		std::cout << "Results received. About to compute norm difference... " << std::endl;

		// Compute norm difference between matrices
		difference = sqrt(computeSquaredNormDifference(oldMatrix, matrix, numRows));

		// Print stuff
		std::cout << "Iteration #" << iter << std::endl;
		std::cout << "Difference: " << difference << std::endl;
		printSquareMatrix(matrix, numRows, "new matrix");
		// printSquareMatrix(oldMatrix, numRows, "old matrix");

		/*char test;
		std::cin >> test;*/
	} while (difference > 1.0e-2 && iter < 100);

	std::cout << "OUT OF THE LOOP" << std::endl;
	for (auto y = 0U; y < numRows - 2; ++y)
	{
		std::cout << "About to send a null buffer";
		MPI_Send(nullptr, 0, MPI_DOUBLE, y + 1, 0, MPI_COMM_WORLD);
		std::cout << "null was sent.";
	}
	std::cout << "THE END";

	delete[] matrix;
	delete[] oldMatrix;
}

void runWorker(uint32_t, uint32_t)
{
	int numProcesses;
	MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);

	// Get matrix rows to process
	auto numRows = numProcesses + 1;
	auto * input = new double[3 * numRows];
	bool jobsDone = false;

	while (!jobsDone)
	{
		MPI_Recv(input, 3 * numRows, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		//std::cout << *(input - (3 * numRows));

		// Null input means computation is over
		if (input == nullptr)
		{
			std::cout << "input null !";
			jobsDone = true;
			return;
		}

		// Computation
		auto * result = new double[numRows - 2];
		for (auto i = 1U; i < numRows - 1; ++i)
		{
			result[i - 1] = (input[i] +
				input[numRows + i + 1] +
				input[2 * numRows + i] +
				input[numRows + i - 1]) / 4.0;
		}

		// Send result back
		MPI_Send(result, numRows - 2, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);

	}
	delete[] input;
}

int main(int argc, char ** argv)
{
    runManagerWorkerAlgorithm(argc, argv, runManager, runWorker);
}