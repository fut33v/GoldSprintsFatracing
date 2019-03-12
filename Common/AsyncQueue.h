// Copyright 2018

#ifndef COMMON_ASYNC_QUEUE_H_
#define COMMON_ASYNC_QUEUE_H_

#include "BaseThread.h"
#include <queue>
#include <mutex>
#include <condition_variable>

namespace VMM
{
	// TODO(victor) FAQ для тупых, как правильно пользоваться
    template<typename T>
    class AsyncQueue : public BaseThread
    {
        std::atomic<bool> mStopQueue {false};
        std::queue<std::shared_ptr<T>> m_queue;
        std::mutex m_queueMutex;
        std::condition_variable mNotify;

    public:
        AsyncQueue() = default;
		AsyncQueue(AsyncQueue&& other) /*noexcept*/ = default;

        AsyncQueue(const AsyncQueue&) = delete;
        AsyncQueue& operator=(const AsyncQueue&) = delete;

        virtual ~AsyncQueue();

        bool AddItem(std::shared_ptr<T> aItem, size_t aMaxQueueSize = 0);
        bool AddItemSkip(std::shared_ptr<T> aItem, size_t aMaxQueueSize = 0);
        bool AddItem(T* aItem, size_t aMaxQueueSize = 0);

        void ClearQueue();
        virtual bool StartThread() override;
        virtual void StopThread() override;

    protected:
        virtual void ThreadFunc() override;

        /// procesing items from queue
        virtual void HandleWorkItem(std::shared_ptr<T> aItem) = 0;

        virtual bool IsThreadActive() override;

    private:
        std::shared_ptr<T> GetNextWorkItem();
    };   

    template<class T>
    inline AsyncQueue<T>::~AsyncQueue()
    {
        mStopQueue = true;
        mNotify.notify_all();  
    }

    template<class T>
    inline bool AsyncQueue<T>::AddItem(std::shared_ptr<T> aItem, size_t aMaxQueueSize)
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        if (aMaxQueueSize > 0)
        {
            const size_t requiredQueueSize = static_cast<size_t>(aMaxQueueSize - 1);

            // Check queue size and remove "old" elements from queue's begin
            while (m_queue.size() > requiredQueueSize)
            {
                m_queue.pop();
            }
        }

        m_queue.push(aItem);
        mNotify.notify_all();
        return true;
    }

    template<class T>
    inline bool AsyncQueue<T>::AddItemSkip(std::shared_ptr<T> aItem, size_t aMaxQueueSize)
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        if (aMaxQueueSize > 0)
        {
            const size_t requiredQueueSize = static_cast<size_t>(aMaxQueueSize - 1);

            if (m_queue.size() > requiredQueueSize)
            {
                return false;
            }
        }

        m_queue.push(aItem);
        mNotify.notify_all();
        return true;
    }

    template<class T>
    inline bool AsyncQueue<T>::AddItem(T* aItem, size_t aMaxQueueSize)
    {
        return AddItem(std::shared_ptr<T>(aItem), aMaxQueueSize);
    }

    template<class T>
    inline void AsyncQueue<T>::ClearQueue()
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_queue = std::queue<std::shared_ptr<T>>();
    }

    template<class T>
    inline void AsyncQueue<T>::ThreadFunc()
    {
        while (IsThreadActive())
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            while (IsThreadActive() && m_queue.empty())
            {
                // loop to avoid spurious wakeups
                mNotify.wait(lock);
            }
            lock.unlock();
            
            std::shared_ptr<T> workItem = GetNextWorkItem();
            while (IsThreadActive() && workItem)
            {
                try
                {
                    HandleWorkItem(workItem);
                }
                catch (...) {}
                workItem = GetNextWorkItem();
            }
        }
    }

    template<class T>
    inline bool AsyncQueue<T>::StartThread()
    {
        mStopQueue = false;
        return BaseThread::StartThread();
    }

    template<class T>
    inline void AsyncQueue<T>::StopThread()
    {
        mStopQueue = true;
        mNotify.notify_all();
        BaseThread::StopThread();
    }

    template<class T>
    inline bool AsyncQueue<T>::IsThreadActive()
    {
        return !mStopQueue && BaseThread::IsThreadActive();
    }

    template<class T>
    inline std::shared_ptr<T> AsyncQueue<T>::GetNextWorkItem()
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        std::shared_ptr<T> workItem;
        if (!m_queue.empty())
        {
            workItem = m_queue.front();
            m_queue.pop();
        }
        return workItem;
    }
}

#endif

