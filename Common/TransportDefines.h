// Copyright 2017 Pelengator

#ifndef SUBPRJ_COMMON_BASE_COMMUNICATION_TRANSPORTDEFINES_H_
#define SUBPRJ_COMMON_BASE_COMMUNICATION_TRANSPORTDEFINES_H_

#include <cstdint>
#include <vector>

#include <boost/asio/serial_port_base.hpp>


namespace VMM {

	const std::string LOOP_ADDRESS = "127.0.0.1";

	//! Тип сообщения в Ethernet
	enum class EthernetBroadcastType : uint8_t {
		unicast,
		multicast,
		broadcast
	};

	//! Тип транспорта для камеры
	enum class TransportType : uint8_t {
		//! Езернет
		Ethernet,
		//! Последовательный порт
		Serial,
		//! Видео
		VideoFile,
		//! Папка с изображениями
		PicturesDir,
		//! Если для инициализации нужен только ID
		UID
	};

	//! Строковые названия для типов транспорта камеры
	class StringsTransportType {
	public:
        static constexpr const char* Ethernet = "Ethernet";
        static constexpr const char* Serial = "Serial";
        static constexpr const char* VideoFile = "VideoFile";

		std::vector<std::string> strings = { Ethernet, Serial, VideoFile};
	};

	//! Тип протокола IP
	enum class IPProtocolType : uint8_t {
		TCP,
		UDP
	};

	//! Структура настроек последовательного порта Boost
	struct SerialPortSettings {
		//! Имя порта
		std::string PortName = "";
		//! Битрейт
		uint32_t BaudRate = 0u;
		//! Размер символа
		uint32_t CharacterSize = 0u;
		//! Чётность
		boost::asio::serial_port_base::parity::type Parity = boost::asio::serial_port_base::parity::type::none;
		//! Стоп-бит
		boost::asio::serial_port_base::stop_bits::type StopBits = boost::asio::serial_port_base::stop_bits::type::one;
		//! Управление потоком
		boost::asio::serial_port_base::flow_control::type FlowControl = boost::asio::serial_port_base::flow_control::type::none;

		//! Из настроек есть только имя порта 
		bool PortOnly;

		//! Имя xml ноды с именем порта
        static constexpr const char* PORT_NAME_ATTRIBUTE = "PortName";
		//! Имя xml ноды с битрейтом
        static constexpr const char* BAUD_RATE_ATTRIBUTE = "BaudRate";
		//! Имя xml ноды с размером символа
        static constexpr const char* CHARACTER_SIZE_ATTRIBUTE = "CharacterSize";
		//! Имя xml ноды с чётностью
        static constexpr const char* PARITY_ATTRIBUTE = "Parity";
		//! Имя xml ноды с стоп-битом
        static constexpr const char* STOP_BITS_ATTRIBUTE = "StopBits";
		//! Имя xml ноды с управлением потоком
        static constexpr const char* FLOW_CONTROL_ATTRIBUTE = "FlowControl";

        static constexpr const char* NONE_VALUE = "none";
        static constexpr const char* ONE_VALUE = "one";
        static constexpr const char* TWO_VALUE = "two";
        static constexpr const char* ONEPOINTFIVE_VALUE = "onepointfive";
        static constexpr const char* EVEN_VALUE = "even";
        static constexpr const char* ODD_VALUE = "odd";
        static constexpr const char* HARDWARE_VALUE = "hardware";
        static constexpr const char* SOFTWARE_VALUE = "software";

		bool operator==(const SerialPortSettings& other) const
		{
			if (PortOnly)
			{
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

		size_t GetSize()
		{
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

	//! Структура настроек Ethernet
	struct EthernetSettings {
		//! Тип вещания
		EthernetBroadcastType broadcastType = EthernetBroadcastType::unicast;

		//! тип  протокола IP
		IPProtocolType protocol = IPProtocolType::TCP;

		//! IP адрес
		std::string IP = "";

		//! порт
		uint16_t Port = 0u;

        static constexpr const char* BROADCAST_TYPE_ATTRIBUTE = "BroadcastType";
        static constexpr const char* UNICAST_VALUE = "unicast";
        static constexpr const char* MULTICAST_VALUE = "multicast";
        static constexpr const char* BROADCAST_VALUE = "broadcast";
        static constexpr const char* PROTOCOL_ATTRIBUTE = "Protocol";
        static constexpr const char* TCP_VALUE = "TCP";
        static constexpr const char* UDP_VALUE = "UDP";
        static constexpr const char* IP_ATTRIBUTE = "IP";
        static constexpr const char* PORT_ATTRIBUTE = "Port";


		bool operator==(const EthernetSettings& other) const {
			return IP == other.IP && Port == other.Port;
		}

		size_t GetSize()
		{
			return				
				sizeof(broadcastType) +
				sizeof(protocol) +
				IP.size() +
				sizeof(Port);
		}
	};



	bool SetSerialPortSettings(boost::asio::serial_port& aSerialPort, const VMM::SerialPortSettings& aSerialPortSettings) 	{
		try {
			aSerialPort.set_option(boost::asio::serial_port_base::baud_rate(aSerialPortSettings.BaudRate));
			aSerialPort.set_option(boost::asio::serial_port_base::character_size(aSerialPortSettings.CharacterSize));
			aSerialPort.set_option(boost::asio::serial_port_base::parity(aSerialPortSettings.Parity));
			aSerialPort.set_option(boost::asio::serial_port_base::stop_bits(aSerialPortSettings.StopBits));
			aSerialPort.set_option(boost::asio::serial_port_base::flow_control(aSerialPortSettings.FlowControl));
		} catch (std::exception& e) {
			LOGGER_LOG(VMM::PriorityEnum::Error, "Не удалось установить настройки последовательного порта");
			LOGGER_LOG(VMM::PriorityEnum::Error, e.what());
			return false;
		}
		return true;
	}


} // namespace AFA

#endif // SUBPRJ_COMMON_BASE_COMMUNICATION_TRANSPORTDEFINES_H_
