#include <assert.h>
#include <mpi.h>
#include <stdint.h>

class MasterWorkerMPIApp
{
public:

    MasterWorkerMPIApp(int argc, char ** argv)
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

    void run()
    {
        // Execute appropriate code depending on process ID
        isMasterProcess() ? runMaster() : runWorker();
    }

    virtual ~MasterWorkerMPIApp()
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

    static uint32_t numProcesses()
    {
        return s_NumProcesses;
    }

    static uint32_t currentProcessID()
    {
        return s_CurrProcessID;
    }


private:

    static uint32_t s_NumProcesses;
    static uint32_t s_CurrProcessID;

    static bool isMasterProcess()
    {
        return currentProcessID() == 0;
    }
};