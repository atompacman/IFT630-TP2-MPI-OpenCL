#include "MPIUtils.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <mpi.h>

//uint32_t    const MATRIX_SIZE = 20;
//uint32_t    const NUM_ELEM = MATRIX_SIZE * MATRIX_SIZE;
//MPI_Request const ON_VACATION = std::numeric_limits<MPI_Request>().max();

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

//void sendJob(double      * i_MatrixA,
//             double      * i_MatrixBt,
//             double      * i_Results,
//             double      * i_RowsBuf,
//             MPI_Request * i_PendingRequests,
//             uint32_t      i_MatrixComponent,
//             uint32_t      i_ProcessID)
//{
//    // Compute position in matrices where concerned rows are
//    auto row = i_MatrixComponent / MATRIX_SIZE;
//    auto col = i_MatrixComponent % MATRIX_SIZE;
//
//    // Copy matrix rows in the row buffer
//    memcpy(i_RowsBuf, i_MatrixA + row * MATRIX_SIZE, MATRIX_SIZE * sizeof(double));
//    memcpy(i_RowsBuf + MATRIX_SIZE, i_MatrixBt + col * MATRIX_SIZE, MATRIX_SIZE * sizeof(double));
//
//    // Send row buffers to worker
//    MPI_Send(i_RowsBuf, 2 * MATRIX_SIZE, MPI_DOUBLE, i_ProcessID, 0, MPI_COMM_WORLD);
//
//    // Post non-blocking receive to be ready for result reception
//    MPI_Request request;
//    auto *      result = i_Results + i_MatrixComponent;
//    MPI_Irecv(result, 1, MPI_DOUBLE, i_ProcessID, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
//    i_PendingRequests[i_ProcessID - 1] = request;
//}

//void sendOnVacation(uint32_t i_ProcessID, MPI_Request * i_PendingRequests)
//{
//    double dummy[2 * MATRIX_SIZE];
//    MPI_Send(dummy, 2 * MATRIX_SIZE, MPI_DOUBLE, i_ProcessID, ON_VACATION, MPI_COMM_WORLD);
//    i_PendingRequests[i_ProcessID - 1] = ON_VACATION;
//}

void runManager(uint32_t, uint32_t i_NumProc)
{
	int numProcesses;
	MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);

	// Create matrix
	auto numRows = numProcesses + 1;
	auto * matrix = new double[numRows * numRows];

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
		difference = 0.0;
		// Send rows to workers
		for (auto y = 0U; y < numRows - 2; ++y)
		{
			MPI_Send(matrix + y * numRows, 3 * numRows, MPI_DOUBLE, y+1, 0, MPI_COMM_WORLD);
		}

		for (auto y = 1U; y < numRows - 1; ++y)
		{
			MPI_Recv(matrix + 1 + y * numRows, numRows - 2, MPI_DOUBLE, y, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
		std::cout << "Iteration #" << iter << std::endl;
		std::cout << "Difference: " << difference << std::endl;
		printSquareMatrix(matrix, numRows, "");

		//*** Receive the results and deal with them here (and while loop with difference ?) ***
	} while (difference > 1.0e-2);
	delete[] matrix;
}

void runWorker(uint32_t, uint32_t)
{
	int numProcesses;
	MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);

	// Get matrix rows to process
	auto numRows = numProcesses + 1;
	auto * input = new double[3 * numRows];
	MPI_Recv(input, 3 * numRows, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	//std::cout << *(input - (3 * numRows));

	// Null input means computation is over
	if (input == nullptr)
	{
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

	delete[] input;
}

int main(int argc, char ** argv)
{
    runManagerWorkerAlgorithm(argc, argv, runManager, runWorker);
}