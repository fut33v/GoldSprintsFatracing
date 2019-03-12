#ifndef TIME_FORMAT_UNIT_H_
#define TIME_FORMAT_UNIT_H_

#include <stdint.h>
#include <thread>

#include <boost/asio/serial_port.hpp>

#include "CommunicationCommon.h"
#include "TransportDefines.h"


namespace Fatracing {

class TimeFormatUnit {

public:
//	typedef std::function<void(const Message&)> OnReadCallback;
//	typedef std::function<void()> OnWriteCallback;

private:
	boost::asio::io_service mIoService;
	boost::asio::io_service::work mDummyWork;
	boost::asio::serial_port mSerialPort;

	SerialPortSettings mSerialPortSettings;
	bool mStopReadThread;
	std::thread mReadThread;
	Communication::Buffer mBuffer;

	std::vector<OnReadCallback> mOnReadCallbacks;
	std::mutex mCallbackMutex;

	std::chrono::system_clock::time_point mLastReadTime;
	std::mutex mLastReadTimeMutex;
	std::atomic<bool> mReOpenPortStopped{false};
	std::atomic<bool> mAreWeHappy{false};
	std::thread mReOpenPortThread;

public:
	TimeFormatUnit();
	~TimeFormatUnit();

	bool Init(const SerialPortSettings& aSerialPortSettings, bool aReInit);
	void AddOnReadCallback(OnReadCallback aCallback);
	void ClearCallbacks();

private:
	void ReInit(bool aWaitForReadThread);
	void ReadThreadFunc();
	void ReOpenPortFunc();
	bool Write(Communication::Buffer aBuffer);

	static std::vector<Communication::Buffer> SplitPackets(Communication::Buffer data);
};

} // namespace VMM

#endif // TIME_FORMAT_UNIT_H_
