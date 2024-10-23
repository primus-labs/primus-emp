#include "emp-tool/execution/circuit_execution.h"
#include "emp-tool/execution/protocol_execution.h"
#include "emp-tool/utils/utils.h"
#include "emp-tool/utils/ThreadPool.h"

#ifndef THREADING
emp::ProtocolExecution* emp::ProtocolExecution::prot_exec = nullptr;
emp::CircuitExecution* emp::CircuitExecution::circ_exec = nullptr;
#else
__thread emp::ProtocolExecution* emp::ProtocolExecution::prot_exec = nullptr;
__thread emp::CircuitExecution* emp::CircuitExecution::circ_exec = nullptr;
#endif

