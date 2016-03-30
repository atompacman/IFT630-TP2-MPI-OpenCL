#include "OCLWrapper/OCLKernel.h"
#include "OCLWrapper/OCLProgram.h"
#include "Encode.h"

#include <iostream>

int main()
{
    // The word the program looks for
    char const * SOLUTION = "jeremie";

    // Used to determine the number of threads to start
    unsigned int const SQRT_NUM_THREADS = 2 << 3;

    // Length of the word to decode
    unsigned int const MSG_LEN = strlen(SOLUTION);

    // Create OpenCL program (automatically pick first device)
    OCLProgram program;

    // Compile kernel file
    if (!program.CreateProgramFromSourceFile("src/Kernel.cl"))
    {
        std::cerr << "Failed to compile kernel" << std::endl;
        return EXIT_FAILURE;
    }

    // Create kernel function
    OCLKernel& kernel = *program.CreateKernelFunction("main");

    // Create buffers
    OCLBuffer * encodedMsgBuf = program.CreateBuffer("Encoded message", OCLBuffer::READ_ONLY,  MSG_LEN * sizeof(char));
    OCLBuffer * solutionsBuf  = program.CreateBuffer("Solutions",       OCLBuffer::WRITE_ONLY, MSG_LEN * sizeof(char));

    // Bind buffers to kernel
    if (!kernel.SetArgBuffer(0, encodedMsgBuf) ||
        !kernel.SetArgBuffer(1, solutionsBuf))
    {
        std::cerr << "Failed to bind buffers to kernel" << std::endl;
        return EXIT_FAILURE;
    }

    // Write encoded message in input buffer
    program.WriteBuffer(encodedMsgBuf, const_cast<char *>(encode(SOLUTION).c_str()));

    // Execute kernel
	size_t globalWorkSize[3] = { SQRT_NUM_THREADS, SQRT_NUM_THREADS, SQRT_NUM_THREADS };
	size_t localWorkSize [3] = { 8, 8, 8 };
    if (!program.ExecuteKernel(&kernel, 3, globalWorkSize, localWorkSize))
    {
        std::cerr << "Failed to execute kernel" << std::endl;
        return EXIT_FAILURE;
    }

    // Read output buffer
    char solution[MSG_LEN + 1];
    if (!program.ReadBuffer(solutionsBuf, solution))
    {
        std::cerr << "Failed to read output buffer" << std::endl;
        return EXIT_FAILURE;
    }
    solution[MSG_LEN] = 0;

    // Print solution
    std::cout << "Solution: " << const_cast<char const *>(solution) << std::endl;
}
