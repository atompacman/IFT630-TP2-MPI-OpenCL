#include "MPIUtils.h"
#include <mpi.h>

void runManagerWorkerAlgorithm(int argc, char ** argv, MPIFunc i_ManagerFunc, MPIFunc i_WorkerFunc)
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

    // Get ID of current process
    int processID;
    MPI_Comm_rank(MPI_COMM_WORLD, &processID);

    // Run correct code
    (processID == MANAGER_ID ? i_ManagerFunc : i_WorkerFunc)(processID, numProcesses);

    // Finalize MPI
    MPI_Finalized(&flag);
    if (!flag)
    {
        MPI_Finalize();
    }
}