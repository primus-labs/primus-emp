#pragma once

#include "emp-tool/utils/ThreadPool.h"
#include "emp-tool/utils/utils.h"
 
struct AbstractFunctionWrapper {
    public:
    virtual void execute() = 0;
    virtual void catchException(const char* exceptionMsg) = 0;
    virtual ~AbstractFunctionWrapper() {}
};

extern "C" void FunctionSafeRun(AbstractFunctionWrapper *fnWrapper);

struct FunctionWrapper: public AbstractFunctionWrapper {
    public:
    FunctionWrapper(std::function<void()> f, ThreadPool* p) {
        fn = f;
        pool = p;
    }
    void operator()() {
        FunctionSafeRun(this);
    }

    void execute() override {
        fn();
    }

    void catchException(const char* exceptionMsg) override {
        pool->setStopReason(exceptionMsg);
    }

    private:
    std::function<void()> fn;
    ThreadPool* pool;
};

struct FunctionWrapperV2: public AbstractFunctionWrapper {
    public:
    FunctionWrapperV2(std::function<void()> f) {
        fn = f;
    }
    void operator()() {
        FunctionSafeRun(this);
    }

    void execute() override {
        check_exception_msg();
        if (this->exceptionMsg->empty()) {
            fn();
        }
    }

    void catchException(const char* e) override {
        check_exception_msg();
        *exceptionMsg = e;
    }


    static std::string getExceptionMsg() {
        check_exception_msg();
        return *exceptionMsg;
    }
    static void setExceptionMsgPtr(std::string* ptr) {
        exceptionMsg = ptr;
    }

    private:
    static void check_exception_msg() {
        if (exceptionMsg == nullptr) {
            throw std::runtime_error("exception msg not set");
        }
    }

    std::function<void()> fn;
#ifndef THREADING
    static std::string *exceptionMsg;
#else
    static __thread std::string *exceptionMsg;
#endif
};

struct FunctionWrapperV3: public AbstractFunctionWrapper {
    public:
    FunctionWrapperV3(std::function<void()> tryFn, std::function<void(const char*)> catchFn) {
        this->tryFn = tryFn;
        this->catchFn = catchFn;
    }

    void operator()() {
        FunctionSafeRun(this);
    }

    void execute() override {
        tryFn();
    }

    void catchException(const char* exceptionMsg) override {
        catchFn(exceptionMsg);
    }

    private:
    std::function<void()> tryFn;
    std::function<void(const char*)> catchFn;
};

#define CHECK_THREAD_POOL_EXCEPTION(pool)                    \
    if (!(pool)->getExceptionMsg().empty()) {                \
        throw std::runtime_error((pool)->getExceptionMsg()); \
    }
 
#define SAFE_FINALIZE_IO()                  \
    FunctionWrapperV2([this]() {            \
        this->finalizeIO();                 \
    })()

inline void safeInitialize(std::function<void()> initFn) {
    FunctionWrapperV2([&initFn](){
        initFn();
    })();
}

#define CHECK_FINALIZE_IO_EXCEPTION()                                           \
    do {                                                                        \
        string exceptionMsg = FunctionWrapperV2::getExceptionMsg();             \
        if (!exceptionMsg.empty()) {                                            \
            throw std::runtime_error(exceptionMsg);                             \
        }                                                                       \
    } while(false)

#define CHECK_INITIALIZE_EXCEPTION()                                            \
    CHECK_FINALIZE_IO_EXCEPTION()

#define SET_FINALIZE_IO_EXCEPTION(exceptionMsg)                                 \
    do {                                                                        \
        FunctionWrapperV2([](){}).catchException(exceptionMsg);                 \
    } while(false);
