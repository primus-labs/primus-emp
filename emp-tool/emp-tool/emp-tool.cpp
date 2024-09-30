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

void ThreadPoolSafeRun(ThreadPool* pool) {
    try {
        pool->runInLoop();
    }
    catch (std::exception& e) {
        printf("catch in ThreadPoolSafeRun exception\n");
        pool->stopWithReason(e.what());
    }
    catch (...) {
        printf("catch in ThreadPoolSafeRun ...\n");
        pool->stopWithReason("[OtherError]unknown exception");
    }
}
