#pragma once

#include "emp-tool/utils/ThreadPool.h"
 
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


