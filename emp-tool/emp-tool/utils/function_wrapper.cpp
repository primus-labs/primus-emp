#include "emp-tool/utils/function_wrapper.h"

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
