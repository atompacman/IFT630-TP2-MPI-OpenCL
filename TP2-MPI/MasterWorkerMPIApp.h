#include "MPIApp.h"
#include <assert.h>

class MasterWorkerMPIApp : public MPIApp
{
public:

    MasterWorkerMPIApp(int argc, char ** argv) :
        MPIApp(argc, argv)
    {

    }

    void run()
    {
        // Execute appropriate code depending on process ID
        isMasterProcess() ? runMaster() : runWorker();
    }


protected:

    virtual void runMaster() = 0;
    virtual void runWorker() = 0;

    static void sendToWorker(void const * i_Buf, 
                             uint32_t     i_Count, 
                             MPI_Datatype i_Type, 
                             uint32_t     i_WorkerID)
    {
        assert(isMasterProcess());
        MPI_Send(i_Buf, i_Count, i_Type, i_WorkerID, 0, MPI_COMM_WORLD);
    }

    static void receiveFromAnyWorker(void *       i_Buf,
                                     uint32_t     i_Count,
                                     MPI_Datatype i_Type,
                                     MPI_Status * i_Status = MPI_STATUS_IGNORE)
    {
        assert(isMasterProcess());
        MPI_Recv(i_Buf, i_Count, i_Type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, i_Status);
    }

    static void sendToMaster(void const * i_Buf,
                             uint32_t     i_Count,
                             MPI_Datatype i_Type)
    {
        assert(!isMasterProcess());
        MPI_Send(i_Buf, i_Count, i_Type, 0, 0, MPI_COMM_WORLD);
    }

    static void receiveFromMaster(void *       i_Buf,
                                  uint32_t     i_Count,
                                  MPI_Datatype i_Type)
    {
        assert(!isMasterProcess());
        MPI_Recv(i_Buf, i_Count, i_Type, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }


private:

    static bool isMasterProcess()
    {
        return currentProcessID() == 0;
    }
};