#ifndef COMMON_TICK_THREAD_H_
#define COMMON_TICK_THREAD_H_

#include "BaseThread.h"

namespace VMM
{
	typedef std::function<void()> ThreadCallback;

	class TickThread : public BaseThread
	{
		ThreadCallback m_callback;
		mutable std::mutex m_threadMutex;
		int m_intervalMs;

	public:
		TickThread(ThreadCallback aCallback, int aInterval);
		~TickThread();

	private:
		virtual void ThreadFunc() override;
	};

    inline TickThread::TickThread(ThreadCallback aCallback, int aIntervalMs) :
		m_callback(aCallback),
		m_intervalMs(aIntervalMs)
	{
	}

	inline TickThread::~TickThread()
	{
	}

    inline void	TickThread::ThreadFunc()
	{
		while (!m_stopThread)
		{
			std::unique_lock<std::mutex> lock(m_threadMutex);
			bool result = m_notify.wait_for(lock, std::chrono::milliseconds(m_intervalMs), [this] { return m_stopThread.load(); });
			lock.unlock();

			if (!result)
			{
				// launch callback
				m_callback();
			}
		}
	}
}

#endif