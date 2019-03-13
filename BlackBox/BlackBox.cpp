#include "./BlackBox.h"


namespace Fatracing {

BlackBox::BlackBox(): mDummyWork(mIoService), mSerialPort(mIoService), mStopReadThread(false) {
	mBuffer.resize(512);
}

BlackBox::~BlackBox() {
    ClearCallback();

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

bool BlackBox::Init(const SerialPortSettings& aSerialPortSettings) {
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
			return false;
		} else {
			if (aSerialPortSettings.PortOnly) {
                s.BaudRate = 9600;
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
//	mLastReadTime = std::chrono::system_clock::now() + std::chrono::milliseconds(1000);
	mAreWeHappy = true;

	return true;
}

void BlackBox::SetCallback(std::function<void (RacersEnum)> aCallback) {
    std::unique_lock<std::mutex> lock(mCallbackMutex);
    mCallback = aCallback;
}

void BlackBox::ClearCallback() {
    std::unique_lock<std::mutex> lock(mCallbackMutex);
    mCallback = nullptr;
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

        Buffer data(mBuffer.begin(), mBuffer.begin() + bytesReceived);
        if (data.size() == 3) {
            RacersEnum racer;
            bool success = false;
            if (data[0] == 'r') {
                racer = RacersEnum::RED;
                success = true;
            } else if (data[0] == 'l') {
                racer = RacersEnum::BLUE;
                success = true;
            }
            if (success) {
                std::unique_lock<std::mutex> lock(mCallbackMutex);
                if (mCallback) {
                    mCallback(racer);
                }
            }
        }

	}
}


}
