// Copyright 2018

#ifndef COMMON_BASE_THREAD_H_
#define COMMON_BASE_THREAD_H_

#include <thread>
#include <mutex>
#include <atomic>

namespace VMM
{
    class BaseThread
    {
        std::thread mThread;
        std::mutex mMutex;
        std::atomic<bool> mStopThread{true};

    protected:
        BaseThread();
        virtual ~BaseThread();
        
		//! Функция потока, то что выполняется в отдельном потоке 
		//! реализуется классами наследниками
        virtual void ThreadFunc() = 0;

    public:
		//! Запустить поток
		virtual bool StartThread();
		//! Остановить поток
		virtual void StopThread();
		//! Проверка активности потока
        virtual bool IsThreadActive();
    };
}

#endif

