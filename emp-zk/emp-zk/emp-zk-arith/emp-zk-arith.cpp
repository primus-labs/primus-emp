#include "emp-zk/emp-zk-arith/zk_fp_exec.h"

#ifndef THREADING
ZKFpExec * ZKFpExec::zk_exec = nullptr;
#else
__thread ZKFpExec * ZKFpExec::zk_exec = nullptr;
#endif
