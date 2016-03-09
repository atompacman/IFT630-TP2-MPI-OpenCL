#include "mpi.h"
#include <cstring>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int    const MASTER_PROCESS = 0;
double const EPSILON        = 1e-6;

void printSquareMatrix(double * i_Matrix, int i_NumRows, bool i_DoTranspose = false)
{
    for (auto y = 0; y < i_NumRows; ++y)
    {
        for (auto x = 0; x < i_NumRows; ++x)
        {
            double val = i_Matrix[i_DoTranspose ? x * i_NumRows + y : y * i_NumRows + x];
            std::cout << std::setw(6) << val << " ";
        }
        std::cout << std::endl;
    }
}

void master(int i_NumRowsInMatrix)
{
    // Master process create matrices and dispatch work to workers processes
    auto numElem = i_NumRowsInMatrix * i_NumRowsInMatrix;
    auto * matrix = new double[numElem];

    // Create arbitrary matrix
    for (auto i = 0; i < numElem; ++i)
    {
        matrix[i] = static_cast<double>(i);
    }

    // Print input matrices
    printf("A:\n");
    printSquareMatrix(matrix, i_NumRowsInMatrix);
    printf("\nB:\n");
    printSquareMatrix(matrix, i_NumRowsInMatrix, true);

    // Send rows to workers to perform dot products
    auto * buf = new double[2 * i_NumRowsInMatrix];

    for (auto y = 0; y < i_NumRowsInMatrix; ++y)
    {
        memcpy(buf, matrix + y * i_NumRowsInMatrix, i_NumRowsInMatrix * sizeof(double));
        for (auto x = 0; x < i_NumRowsInMatrix; ++x)
        {
            memcpy(buf + i_NumRowsInMatrix, matrix + x * i_NumRowsInMatrix, 
                   i_NumRowsInMatrix * sizeof(double));
            auto dest = x + y * i_NumRowsInMatrix + 1;
            MPI_Send(buf, 2 * i_NumRowsInMatrix, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
        }
    }
    delete[] buf;

    // Receive results
    auto numReceived = 0;
    while (numReceived++ < numElem)
    {
        MPI_Status status;
        double     result;
        MPI_Recv(&result, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        matrix[status.MPI_SOURCE - 1] = result;
    }

    // Print results
    printf("\nResult:\n");
    printSquareMatrix(matrix, i_NumRowsInMatrix);

    delete[] matrix;
}

void worker(uint32_t i_NumRowsInMatrix, int i_ProcessID)
{
    // Get matrix rows for dot product computation
    auto * buf = new double[2 * i_NumRowsInMatrix];
    MPI_Recv(buf, 2 * i_NumRowsInMatrix, MPI_DOUBLE, MASTER_PROCESS, 
             0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Dot product
    double result = 0;
    for (uint32_t i = 0; i < i_NumRowsInMatrix; ++i)
    {
        result += buf[i] * buf[i + i_NumRowsInMatrix];
    }

    delete[] buf;

    // Send result back
    MPI_Send(&result, 1, MPI_DOUBLE, MASTER_PROCESS, i_ProcessID, MPI_COMM_WORLD);
}

int main(int argc, char ** argv)
{
    // Initialize MPI
    MPI_Init(&argc, &argv);

    // Get number of running processes
    int numProcesses;
    MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);

    // Find the number of rows in the matrix so that all processes have work
    auto numRows = static_cast<uint32_t>(rint(sqrt(numProcesses - 1)));
    if (numRows * numRows != numProcesses - 1)
    {
        printf("ERROR: Expected a number of process that is square + 1, but had %d\n",numProcesses);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    // Get ID of current process
    int processID;
    MPI_Comm_rank(MPI_COMM_WORLD, &processID);
    
    // Master process runs different code
    if (processID == MASTER_PROCESS)
    {
        master(numRows);
    }
    else
    {
        worker(numRows, processID);
    }
    
    // Bye bye
    MPI_Finalize();
}