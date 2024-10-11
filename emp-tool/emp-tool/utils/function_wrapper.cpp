#include "emp-tool/utils/function_wrapper.h"

#ifndef THREADING
std::string* FunctionWrapperV2::exceptionMsg = nullptr;
#else
__thread std::string* FunctionWrapperV2::exceptionMsg = nullptr;
#endif

void FunctionSafeRun(AbstractFunctionWrapper *fnWrapper) {
    try {
        fnWrapper->execute();
    }
    catch (std::exception& e) {
        fnWrapper->catchException(e.what());
    }
    catch (...) {
        fnWrapper->catchException("[OtherError]unknown reason");
    }

}
