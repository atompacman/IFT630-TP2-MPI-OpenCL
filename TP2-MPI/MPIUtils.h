#ifndef MPIUTILS
#define MPIUTILS

#include <functional>
#include <mpi.h>
#include <stdint.h>

namespace MPIUtils
{
    typedef std::function<void(uint32_t i_CurrProc, uint32_t i_NumProc)> MPIFunc;

    void runMasterWorkerAlgorithm(int argc,char** argv, MPIFunc i_MasterFunc, MPIFunc i_WorkerFunc);

    void sendToWorker(void const * i_Buf,
                      uint32_t     i_Count,
                      MPI_Datatype i_Type,
                      uint32_t     i_WorkerID);

    void receiveFromAnyWorker(void *       i_Buf,
                              uint32_t     i_Count,
                              MPI_Datatype i_Type,
                              MPI_Status * i_Status = MPI_STATUS_IGNORE);

    void sendToMaster(void const * i_Buf,
                      uint32_t     i_Count,
                      MPI_Datatype i_Type);

    void receiveFromMaster(void *       i_Buf,
                           uint32_t     i_Count,
                           MPI_Datatype i_Type);
}

#endif //MPIUTILS
