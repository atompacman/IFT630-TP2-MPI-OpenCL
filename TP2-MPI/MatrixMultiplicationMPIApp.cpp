#include "MPIUtils.h"
#include <iomanip>
#include <iostream>

uint32_t const MATRIX_SIZE = 3;

void printSquareMatrix(double *     i_Matrix,
                       int          i_NumRows,
                       char const * i_Name,
                       bool         i_DoTranspose = false)
{
    std::cout << i_Name << ":" << std::endl;
    for (auto y = 0; y < i_NumRows; ++y)
    {
        for (auto x = 0; x < i_NumRows; ++x)
        {
            auto val = i_Matrix[i_DoTranspose ? x * i_NumRows + y : y * i_NumRows + x];
            std::cout << std::setw(6) << val << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void runMaster(uint32_t, uint32_t i_NumProc)
{
    // Create matrix
    auto numElem = MATRIX_SIZE * MATRIX_SIZE;
    auto * matrix = new double[numElem];
    for (auto i = 0U; i < numElem; ++i)
    {
        matrix[i] = static_cast<double>(i);
    }

    // Print input matrices
    printSquareMatrix(matrix, MATRIX_SIZE, "A");
    printSquareMatrix(matrix, MATRIX_SIZE, "B", true);

    // Send rows to workers to perform dot products
    auto * buf = new double[2 * MATRIX_SIZE];

    for (auto y = 0U; y < MATRIX_SIZE; ++y)
    {
        memcpy(buf, matrix + y * MATRIX_SIZE, MATRIX_SIZE * sizeof(double));
        for (auto x = 0U; x < MATRIX_SIZE; ++x)
        {
            memcpy(buf + MATRIX_SIZE, matrix + x * MATRIX_SIZE,
                MATRIX_SIZE * sizeof(double));
            MPIUtils::sendToWorker(buf, 2 * MATRIX_SIZE, MPI_DOUBLE, x + y * MATRIX_SIZE + 1);
        }
    }
    delete[] buf;

    // Receive results
    auto numReceived = 0U;
    while (numReceived++ < numElem)
    {
        MPI_Status status;
        double     result;
        MPIUtils::receiveFromAnyWorker(&result, 1, MPI_DOUBLE, &status);
        matrix[status.MPI_SOURCE - 1] = result;
    }

    // Print results
    printSquareMatrix(matrix, MATRIX_SIZE, "A * B");

    delete[] matrix;
}

void runWorker(uint32_t, uint32_t)
{
    // Get matrix rows for dot product computation
    auto * buf = new double[2 * MATRIX_SIZE];
    MPIUtils::receiveFromMaster(buf, 2 * MATRIX_SIZE, MPI_DOUBLE);

    // Dot product
    auto result = 0.0;
    for (auto i = 0U; i < MATRIX_SIZE; ++i)
    {
        result += buf[i] * buf[i + MATRIX_SIZE];
    }

    delete[] buf;

    // Send result back
    MPIUtils::sendToMaster(&result, 1, MPI_DOUBLE);
}

int main(int argc, char ** argv)
{
    MPIUtils::runMasterWorkerAlgorithm(argc, argv, runMaster, runWorker);
}