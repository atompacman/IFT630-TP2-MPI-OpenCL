#include "mpi.h"
#include <cstring>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

int    const MASTER_PROCESS = 0;
double const EPSILON        = 1e-6;

void master(uint32_t i_NumRowsInMatrix)
{
    // Master process create matrices and dispatch work to workers processes
    auto numElem = i_NumRowsInMatrix * i_NumRowsInMatrix;
    auto * matrix = new double[numElem];

    // Create arbitrary matrix
    for (uint32_t i = 0; i < numElem; ++i)
    {
        matrix[i] = i;
    }

    // Send rows to workers to perform dot products
    auto * buf = new double[2 * i_NumRowsInMatrix];

    for (uint32_t y = 0; y < i_NumRowsInMatrix; ++y)
    {
        for (uint32_t x = 0; x < i_NumRowsInMatrix; ++x)
        {
            // Send two matrix rows in one buffer (a row from matrix A and one for B)
            memcpy(buf,                     matrix + y * i_NumRowsInMatrix, i_NumRowsInMatrix);
            memcpy(buf + i_NumRowsInMatrix, matrix + x * i_NumRowsInMatrix, i_NumRowsInMatrix);
            auto processID = x + y * i_NumRowsInMatrix;
            MPI_Send(buf, 2 * i_NumRowsInMatrix, MPI_DOUBLE, processID, processID, MPI_COMM_WORLD);
        }
    }

    delete[] matrix;
    delete[] buf;
}

void worker()
{
    
}

int main(int argc, char ** argv)
{
    // Initialize MPI
    MPI_Init(&argc, &argv);

    // Get number of running processes
    int numProcesses;
    MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);

    // Find the number of rows in the matrix so that all processes have work
    uint32_t numRows = rint(sqrt(numProcesses - 1));
    if (numRows * numRows != numProcesses - 1)
    {
        printf("ERROR: Expected a number of process that is square + 1, but had %d", numProcesses);
    }

    // Get ID of current process
    int processID;
    MPI_Comm_rank(MPI_COMM_WORLD, &processID);
    
    // Master process runs different code
    if (processID == MASTER_PROCESS)
        master(numRows);
    else
        worker();

    MPI_Finalize();
}