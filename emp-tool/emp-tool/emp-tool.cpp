#include "emp-tool/execution/circuit_execution.h"
#include "emp-tool/execution/protocol_execution.h"
#include "emp-tool/utils/utils.h"
#include "emp-tool/utils/ThreadPool.h"

#ifndef THREADING
emp::ProtocolExecution* emp::ProtocolExecution::prot_exec = nullptr;
emp::CircuitExecution* emp::CircuitExecution::circ_exec = nullptr;
int emp::runtime_errno = 0;
#else
__thread emp::ProtocolExecution* emp::ProtocolExecution::prot_exec = nullptr;
__thread emp::CircuitExecution* emp::CircuitExecution::circ_exec = nullptr;
__thread int emp::runtime_errno = 0;
#endif

void FunctionSafeRun(void *wrapper) {
    FunctionWrapper* fnWrapper = (FunctionWrapper*)wrapper;
    ThreadPool* pool = fnWrapper->getPool();
    try {
        fnWrapper->execute();
    }
    catch (std::exception& e) {
        printf("stopwithreason:%s\n", e.what());
        pool->stopWithReason(e.what());
    }
    catch (...) {
        printf("stopwithreason:%s\n", "unknown reason");
        pool->stopWithReason("[OtherError]unknown reason");
    }

}
