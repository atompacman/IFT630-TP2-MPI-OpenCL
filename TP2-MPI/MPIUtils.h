#ifndef MPIUTILS
#define MPIUTILS

#include <functional>
#include <stdint.h>

uint32_t const MANAGER_ID = 0;

typedef std::function<void(uint32_t i_CurrProc, uint32_t i_NumProc)> MPIFunc;

void runManagerWorkerAlgorithm(int argc, char ** argv, MPIFunc i_ManagerFunc, MPIFunc i_WorkerFunc);

#endif //MPIUTILS