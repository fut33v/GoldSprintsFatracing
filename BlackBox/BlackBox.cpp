#include "BlackBox.h"

namespace Fatracing {

bool BlackBox::Message::Parse(const Communication::Buffer& aBytes) {
	if (aBytes.size() < MESSAGE_SIZE) {
		return false;
	}

	int i = 0;
	if (aBytes[i] != 0xAC || aBytes[i+1] != 0xF8) {
		return false;
	}
	i+=2;

	Version = aBytes[i];
	i++;

	Year = aBytes[i];
	i++;

	Month = aBytes[i];
	i++;

	Day = aBytes[i];
	i++;

	Hour = aBytes[i];
	i++;

	Minute = aBytes[i];
	i++;

	Second = aBytes[i];
	i++;

	Shift = aBytes[i];
	i++;

	LatitudeNorthSouth = aBytes[i];
	i++;

	LatitudeDegrees = aBytes[i];
	i++;

	LatitudeMinutes = aBytes[i];
	i++;

	LatitudeSeconds = aBytes[i];
	i++;

	LatitudeSecondsParts = aBytes[i];
	i++;

	LongitudeEastWest = aBytes[i];
	i++;

	LongitudeDegrees = aBytes[i];
	i++;

	LongitudeMinutes = aBytes[i];
	i++;

	LongitudeSeconds = aBytes[i];
	i++;

	LongitudeSecondsParts = aBytes[i];
	i++;

	return true;
}


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

void BlackBox::AddOnReadCallback(OnReadCallback aCallback) {
	std::unique_lock<std::mutex> lock(mCallbackMutex);
	mOnReadCallbacks.push_back(aCallback);
}

void BlackBox::ClearCallbacks() {
	std::unique_lock<std::mutex> lock(mCallbackMutex);
	mOnReadCallbacks.clear();
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

		Communication::Buffer data(mBuffer.begin(), mBuffer.begin() + bytesReceived);
		std::vector<Communication::Buffer> packets = SplitPackets(data);
		std::vector<Communication::Buffer> unstuffedPackets;
		for (auto p : packets) {
			Communication::Buffer unstuffed(p.size());
			UnStuffData(p.data(), p.size(), unstuffed.data());
			unstuffedPackets.push_back(unstuffed);
		}

		for (auto& p : unstuffedPackets) {
			Message message;
			message.Parse(p);
			for (auto& x : mOnReadCallbacks) {
				x(message);
			}
		}
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

bool BlackBox::Write(Communication::Buffer aBuffer) {
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


std::vector<Communication::Buffer> BlackBox::SplitPackets(Communication::Buffer data) {
	std::vector<Communication::Buffer> packets;
	if (data.size() < MININUM_PACKET_SIZE) {
		return packets;
	}

	for (size_t i = 0; i < data.size(); ++i) {
		if (data[i] == MARKER_FIRST_BYTE && data[i+1]==MARKER_SECOND_BYTE) {
//			for (size_t x = i + 1; x < data.size() - 1; ++x) {
//				if (data[x] != DLE && data[x + 1] == ETX) {
//					Communication::Buffer payload = Communication::Buffer(data.begin() + i + 1, data.begin() + x + 1);
//					packets.push_back(payload);
//					break;
//				}
//			}
		}
	}

	return packets;
}

}
