#include "MPIUtils.h"

namespace MPIUtils
{
    void runMasterWorkerAlgorithm(int argc, char** argv, MPIFunc i_MasterFunc, MPIFunc i_WorkerFunc)
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
        (processID == 0 ? i_MasterFunc : i_WorkerFunc)(processID, numProcesses);

        // Finalize MPI
        MPI_Finalized(&flag);
        if (!flag)
        {
            MPI_Finalize();
        }
    }

    void sendToWorker(void const * i_Buf,
                      uint32_t     i_Count,
                      MPI_Datatype i_Type,
                      uint32_t     i_WorkerID)
    {
        MPI_Send(i_Buf, i_Count, i_Type, i_WorkerID, 0, MPI_COMM_WORLD);
    }

    void receiveFromAnyWorker(void *       i_Buf,
                              uint32_t     i_Count,
                              MPI_Datatype i_Type,
                              MPI_Status * i_Status)
    {
        MPI_Recv(i_Buf, i_Count, i_Type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, i_Status);
    }

    void sendToMaster(void const * i_Buf,
                      uint32_t     i_Count,
                      MPI_Datatype i_Type)
    {
        MPI_Send(i_Buf, i_Count, i_Type, 0, 0, MPI_COMM_WORLD);
    }

    void receiveFromMaster(void *       i_Buf,
                           uint32_t     i_Count,
                           MPI_Datatype i_Type)
    {
        MPI_Recv(i_Buf, i_Count, i_Type, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

}