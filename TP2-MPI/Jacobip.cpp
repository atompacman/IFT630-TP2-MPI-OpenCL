#include "MasterWorkerMPIApp.h"
#include <iomanip>
#include <iostream>

class JacobiMPIApp : public MasterWorkerMPIApp
{
public:

    JacobiMPIApp(int argc, char ** argv) :
        MasterWorkerMPIApp(argc, argv)
    {

    }


protected:

    void runMaster() override
    {
        // Create matrix
        auto numRows = numProcesses() + 1;
        auto * matrix = new double[numRows * numRows];

        for (auto y = 0U; y < numRows; ++y)
        {
            matrix[y * numRows              ] = -1.0;
            matrix[y * numRows + numRows - 1] = -1.0;
            auto rowValue = y == 0 || y == numRows - 1 ? -1.0 : y;
            for (auto x = 1U; x < numRows - 1; ++x)
            {
                matrix[y * numRows + x] = rowValue;
            }
        }

        // Print matrix
        printSquareMatrix(matrix, numRows, "Initial");

        // Send rows to workers
        for (auto y = 0U; y < numRows - 2; ++y)
        {
            sendToWorker(matrix + y * numRows, 3 * numRows, MPI_DOUBLE, y);
        }

        delete[] matrix;
    }

    void runWorker() override
    {
        // Get matrix rows to process
        auto numRows = numProcesses() + 1;
        auto * input = new double[3 * numRows];
        receiveFromMaster(input, 3 * numRows, MPI_DOUBLE);

        // Null input means computation is over
        if (input == nullptr)
        {
            return;
        }

        // Computation
        auto * result = new double[numRows - 2];
        for (auto i = 1U; i < numRows - 1; ++i)
        {
            result[i - 1] = (input[i              ] + 
                             input[numRows + i + 1] + 
                             input[2 * numRows + i] + 
                             input[numRows + i - 1]) / 4.0;
        }

        // Send result back
        sendToMaster(&result, numRows - 2,  MPI_DOUBLE);

        delete[] input;
    }


private:

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
};

int main2(int argc, char ** argv)
{
    JacobiMPIApp(argc, argv).run();
    return EXIT_SUCCESS;
}