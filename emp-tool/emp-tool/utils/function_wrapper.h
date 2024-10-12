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
        if (this->exceptionMsg == nullptr) {
            fn();
        }
    }

    void catchException(const char* exceptionMsg) override {
        if (this->exceptionMsg != nullptr) {
            delete this->exceptionMsg;
            this->exceptionMsg = nullptr;
        }
        this->exceptionMsg = new std::string(exceptionMsg);
    }

    static std::string getExceptionMsgOnce() {
        if (exceptionMsg == nullptr) {
            return "";
        }
        std::string e = *exceptionMsg;
        delete exceptionMsg;
        exceptionMsg = nullptr;

        return e;
    }

    private:
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

#define CHECK_FINALIZE_IO_EXCEPTION()                                           \
    do {                                                                        \
        string exceptionMsg = FunctionWrapperV2::getExceptionMsgOnce();         \
        if (!exceptionMsg.empty()) {                                            \
            throw std::runtime_error(exceptionMsg);                             \
        }                                                                       \
    } while(false)

#define SET_FINALIZE_IO_EXCEPTION(exceptionMsg)                                 \
    do {                                                                        \
        FunctionWrapperV2([](){}).catchException(exceptionMsg);                 \
    } while(false);
