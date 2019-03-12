#include "BaseThread.h"

namespace Fatracing
{
    BaseThread::BaseThread()
    {
    }

    BaseThread::~BaseThread()
    {
        BaseThread::StopThread();
    }

    bool
    BaseThread::StartThread()
    {
        std::unique_lock<std::mutex> lock(mMutex);
        if (mStopThread)
        {
            // Wait for previous thread finish
            if (mThread.joinable()) mThread.join();

            // Launch new thread
            mStopThread = false;
            mThread = std::thread([&]
                {
                    ThreadFunc();
                    mStopThread = true;
                });

            return true;
        }
        return false;
    }

    void
    BaseThread::StopThread()
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mStopThread = true;
        if (mThread.joinable()) mThread.join();
    }

    bool
    BaseThread::IsThreadActive()
    {
        return !mStopThread;
    }
}
