#ifndef TIME_FORMAT_UNIT_H_
#define TIME_FORMAT_UNIT_H_

#include <stdint.h>
#include <thread>
#include <mutex>

#include <boost/asio/serial_port.hpp>

#include "Logger.h"


namespace Fatracing {

struct SerialPortSettings {
    std::string PortName = "";
    uint32_t BaudRate = 0u;
    uint32_t CharacterSize = 0u;
    boost::asio::serial_port_base::parity::type Parity = boost::asio::serial_port_base::parity::type::none;
    boost::asio::serial_port_base::stop_bits::type StopBits = boost::asio::serial_port_base::stop_bits::type::one;
    boost::asio::serial_port_base::flow_control::type FlowControl = boost::asio::serial_port_base::flow_control::type::none;

    bool PortOnly;

    static constexpr const char* PORT_NAME_ATTRIBUTE = "PortName";
    static constexpr const char* BAUD_RATE_ATTRIBUTE = "BaudRate";
    static constexpr const char* CHARACTER_SIZE_ATTRIBUTE = "CharacterSize";
    static constexpr const char* PARITY_ATTRIBUTE = "Parity";
    static constexpr const char* STOP_BITS_ATTRIBUTE = "StopBits";
    static constexpr const char* FLOW_CONTROL_ATTRIBUTE = "FlowControl";

    static constexpr const char* NONE_VALUE = "none";
    static constexpr const char* ONE_VALUE = "one";
    static constexpr const char* TWO_VALUE = "two";
    static constexpr const char* ONEPOINTFIVE_VALUE = "onepointfive";
    static constexpr const char* EVEN_VALUE = "even";
    static constexpr const char* ODD_VALUE = "odd";
    static constexpr const char* HARDWARE_VALUE = "hardware";
    static constexpr const char* SOFTWARE_VALUE = "software";

    bool operator==(const SerialPortSettings& other) const {
        if (PortOnly) {
            return PortName == other.PortName;
        }
        return
                PortName == other.PortName &&
                BaudRate == other.BaudRate &&
                CharacterSize == other.CharacterSize &&
                Parity == other.Parity &&
                StopBits == other.StopBits &&
                FlowControl == other.FlowControl;
    }

    size_t GetSize() {
        return
                PortName.size() +
                sizeof(BaudRate) +
                sizeof(CharacterSize) +
                sizeof(Parity) +
                sizeof(StopBits) +
                sizeof(FlowControl) +
                sizeof(PortOnly);
    }
};


inline bool SetSerialPortSettings(boost::asio::serial_port& aSerialPort, const SerialPortSettings& aSerialPortSettings) 	{
    try {
        aSerialPort.set_option(boost::asio::serial_port_base::baud_rate(aSerialPortSettings.BaudRate));
        aSerialPort.set_option(boost::asio::serial_port_base::character_size(aSerialPortSettings.CharacterSize));
        aSerialPort.set_option(boost::asio::serial_port_base::parity(aSerialPortSettings.Parity));
        aSerialPort.set_option(boost::asio::serial_port_base::stop_bits(aSerialPortSettings.StopBits));
        aSerialPort.set_option(boost::asio::serial_port_base::flow_control(aSerialPortSettings.FlowControl));
    } catch (std::exception& e) {
        LOGGER_LOG(PriorityEnum::Error, "Не удалось установить настройки последовательного порта");
        LOGGER_LOG(PriorityEnum::Error, e.what());
        return false;
    }
    return true;
}


class BlackBox {

    typedef std::vector<uint8_t> Buffer;
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
    Buffer mBuffer;

//    std::vector<OnReadCallback> mOnReadCallbacks;
    std::mutex mCallbackMutex;

    std::chrono::system_clock::time_point mLastReadTime;
    std::mutex mLastReadTimeMutex;
    std::atomic<bool> mReOpenPortStopped{false};
    std::atomic<bool> mAreWeHappy{false};
    std::thread mReOpenPortThread;

public:
    BlackBox();
    ~BlackBox();

    bool Init(const SerialPortSettings& aSerialPortSettings, bool aReInit=false);
//    void AddOnReadCallback(OnReadCallback aCallback);
    void ClearCallbacks();

private:
    void ReInit(bool aWaitForReadThread);
    void ReadThreadFunc();
    void ReOpenPortFunc();
    bool Write(Buffer aBuffer);

    static std::vector<Buffer> SplitPackets(Buffer data);
};

} // namespace Fatracing

#endif // TIME_FORMAT_UNIT_H_
