#include "mpi.h"
#include <cstdint>
#include <stdlib.h>

class MPIApp
{
public:

    MPIApp(int argc, char ** argv)
    {
        // Initialize MPI
        int flag;
        MPI_Initialized(&flag);
        if (!flag)
        {
            MPI_Init(&argc, &argv);
        }

        // Get number of running processes
        int numProcesses;
        MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
        s_NumProcesses = static_cast<uint32_t>(numProcesses);

        // Get ID of current process
        int processID;
        MPI_Comm_rank(MPI_COMM_WORLD, &processID);
        s_CurrProcessID = static_cast<uint32_t>(processID);
    }

    virtual ~MPIApp()
    {
        // Finalize MPI
        int flag;
        MPI_Finalized(&flag);
        if (!flag)
        {
            MPI_Finalize();
        }
    }


protected:

    static uint32_t numProcesses()
    {
        return s_NumProcesses;
    }

    static uint32_t currentProcessID()
    {
        return s_CurrProcessID;
    }

    static void abort()
    {
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

private:

    static uint32_t s_NumProcesses;
    static uint32_t s_CurrProcessID;
};
