#include "./BlackBox.h"


namespace Fatracing {

BlackBox::BlackBox(): mDummyWork(mIoService), mSerialPort(mIoService), mStopReadThread(false) {
	mBuffer.resize(512);
}

BlackBox::~BlackBox() {
	ClearCallbacks();

	mReOpenPortStopped = true;
	if (mReOpenPortThread.joinable()) {
		mReOpenPortThread.join();
	}

	mStopReadThread = true;
	if (mSerialPort.is_open()) {
		mSerialPort.close();
	}
	if (mReadThread.joinable()) {
		mReadThread.join();
	}

}

bool BlackBox::Init(const SerialPortSettings& aSerialPortSettings, bool aReInit) {
	auto s = aSerialPortSettings;

	if (aSerialPortSettings.PortName.empty()) {
		LOGGER_LOG(PriorityEnum::Error, "Пустое имя последовательного порта");
		return false;
	}

	while (true) {
		boost::system::error_code err;
		mSerialPort.open(aSerialPortSettings.PortName, err);
		if (err) {
			LOGGER_LOG(PriorityEnum::Error, "Ошибка при открытии последовательного порта: \"%s\"", err.message().c_str());
			//if (!aReInit) {
			//	return false;
			//}
			return false;
			// std::this_thread::sleep_for(std::chrono::seconds(5));
		} else {
			if (aSerialPortSettings.PortOnly) {
				s.BaudRate = 115200;
				s.CharacterSize = 8;
				s.StopBits = boost::asio::serial_port_base::stop_bits::type::one;
				s.FlowControl = boost::asio::serial_port_base::flow_control::type::none;
				s.Parity = boost::asio::serial_port_base::parity::type::none;
			}
			if (SetSerialPortSettings(mSerialPort, s)) {
				break;
			}
		}
	}

	mSerialPortSettings = s;

	mStopReadThread = false;
    mReadThread = std::thread(std::bind(&BlackBox::ReadThreadFunc, this));
    if (!aReInit) { mReOpenPortThread = std::thread(std::bind(&BlackBox::ReOpenPortFunc, this)); }
	mLastReadTime = std::chrono::system_clock::now() + std::chrono::milliseconds(1000);
	mAreWeHappy = true;

	return true;
}

//void BlackBox::AddOnReadCallback(OnReadCallback aCallback) {
//	std::unique_lock<std::mutex> lock(mCallbackMutex);
//	mOnReadCallbacks.push_back(aCallback);
//}

void BlackBox::ClearCallbacks() {
//	std::unique_lock<std::mutex> lock(mCallbackMutex);
//	mOnReadCallbacks.clear();
}

void BlackBox::ReInit(bool aWaitForReadThread) {
	mStopReadThread = true;
	if (mSerialPort.is_open()) {
		mSerialPort.close();
	}
	if (mReadThread.joinable() && aWaitForReadThread) {
		mReadThread.join();
	}

	Init(mSerialPortSettings, true);
}

void BlackBox::ReadThreadFunc() {
	while (mSerialPort.is_open()) {
		if (mStopReadThread) {
			return;
		}
		boost::system::error_code err;
		size_t bytesReceived = mSerialPort.read_some(boost::asio::buffer(mBuffer, mBuffer.size()), err);
		if (err) {
			if (mStopReadThread) {
				return;
			}
			LOGGER_LOG(PriorityEnum::Error, "Не удалось прочитать данные из последовательного порта");
			mAreWeHappy = false;
			return;
		}

		std::unique_lock<std::mutex> lock(mLastReadTimeMutex);
		mLastReadTime = std::chrono::system_clock::now();

//        Buffer data(mBuffer.begin(), mBuffer.begin() + bytesReceived);
//        std::vector<Buffer> packets = SplitPackets(data);
//        std::vector<Buffer> unstuffedPackets;
//		for (auto p : packets) {
//            Buffer unstuffed(p.size());
//			UnStuffData(p.data(), p.size(), unstuffed.data());
//			unstuffedPackets.push_back(unstuffed);
//		}

//		for (auto& p : unstuffedPackets) {
//			Message message;
//			message.Parse(p);
//			for (auto& x : mOnReadCallbacks) {
//				x(message);
//			}
//		}
	}
}

void BlackBox::ReOpenPortFunc() {
	while (!mReOpenPortStopped) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		std::unique_lock<std::mutex> lock(mLastReadTimeMutex);
		auto now = std::chrono::system_clock::now();
		auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - mLastReadTime);
		lock.unlock();
		if (diff.count() > 1500 || !mAreWeHappy) {
			ReInit(true);
		}
	}
}

bool BlackBox::Write(Buffer aBuffer) {
	if (!mSerialPort.is_open()) {
		LOGGER_LOG(PriorityEnum::Error, "Последовательный порт не открыт");
		return false;
	}
	boost::system::error_code err;

	size_t bytesTransfered = mSerialPort.write_some(boost::asio::buffer(aBuffer, aBuffer.size()), err);

	LOGGER_LOG(PriorityEnum::Trace, "%zu байт отправлено", bytesTransfered);

	if (err) {
		LOGGER_LOG(PriorityEnum::Error, "Не удалось отправить данные через последовательный порт");
		// ReInit(true);
		// set flag of reinit!!!
		mAreWeHappy = false;
		return false;
	}

	return true;
}


}
