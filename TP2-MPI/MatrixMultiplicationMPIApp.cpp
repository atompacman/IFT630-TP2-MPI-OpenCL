#include "MasterWorkerMPIApp.h"
#include <iomanip>
#include <iostream>

uint32_t MasterWorkerMPIApp::s_NumProcesses;
uint32_t MasterWorkerMPIApp::s_CurrProcessID;

class MatrixMultiplicationMPIApp : public MasterWorkerMPIApp
{
public:
    
    MatrixMultiplicationMPIApp(int argc, char ** argv) :
        MasterWorkerMPIApp(argc, argv)
    {
        // Find the number of rows in the matrix so that all processes have work
        s_NumRows = static_cast<uint32_t>(rint(sqrt(numProcesses() - 1)));
        if (s_NumRows * s_NumRows != numProcesses() - 1)
        {
            std::cerr << "ERROR: Expected a number of process that is a square number + 1, but had " 
                      << numProcesses() << std::endl;
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }


protected:

    void runMaster() override
    {
        // Create matrix
        auto numElem = s_NumRows * s_NumRows;
        auto * matrix = new double[numElem];
        for (auto i = 0U; i < numElem; ++i)
        {
            matrix[i] = static_cast<double>(i);
        }

        // Print input matrices
        printSquareMatrix(matrix, s_NumRows, "A");
        printSquareMatrix(matrix, s_NumRows, "B", true);

        // Send rows to workers to perform dot products
        auto * buf = new double[2 * s_NumRows];

        for (auto y = 0U; y < s_NumRows; ++y)
        {
            memcpy(buf, matrix + y * s_NumRows, s_NumRows * sizeof(double));
            for (auto x = 0U; x < s_NumRows; ++x)
            {
                memcpy(buf + s_NumRows, matrix + x * s_NumRows,
                    s_NumRows * sizeof(double));
                sendToWorker(buf, 2 * s_NumRows, MPI_DOUBLE, x + y * s_NumRows + 1);
            }
        }
        delete[] buf;

        // Receive results
        auto numReceived = 0U;
        while (numReceived++ < numElem)
        {
            MPI_Status status;
            double     result;
            receiveFromAnyWorker(&result, 1, MPI_DOUBLE, &status);
            matrix[status.MPI_SOURCE - 1] = result;
        }

        // Print results
        printSquareMatrix(matrix, s_NumRows, "A * B");

        delete[] matrix;
    }

    void runWorker() override
    {
        // Get matrix rows for dot product computation
        auto * buf = new double[2 * s_NumRows];
        receiveFromMaster(buf, 2 * s_NumRows, MPI_DOUBLE);

        // Dot product
        auto result = 0.0;
        for (auto i = 0U; i < s_NumRows; ++i)
        {
            result += buf[i] * buf[i + s_NumRows];
        }

        delete[] buf;

        // Send result back
        sendToMaster(&result, 1, MPI_DOUBLE);
    }


private:

    static uint32_t s_NumRows;

    static void printSquareMatrix(double *     i_Matrix, 
                                  int          i_NumRows, 
                                  char const * i_Name, 
                                  bool         i_DoTranspose = false)
    {
        std::cout << i_Name << ":" << std::endl;
        for (auto y = 0; y < i_NumRows; ++y)
        {
            for (auto x = 0; x < i_NumRows; ++x)
            {
                double val = i_Matrix[i_DoTranspose ? x * i_NumRows + y : y * i_NumRows + x];
                std::cout << std::setw(6) << val << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
};

uint32_t MatrixMultiplicationMPIApp::s_NumRows;

int main(int argc, char ** argv)
{
    MatrixMultiplicationMPIApp(argc, argv).run();
    return EXIT_SUCCESS;
}