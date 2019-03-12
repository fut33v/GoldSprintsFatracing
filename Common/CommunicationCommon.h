// Copyright 2018

#ifndef VISM_MAIN_SUBPRJ_COMMON_COMMUNICATION_COMMUNICATIONCOMMON_H_
#define VISM_MAIN_SUBPRJ_COMMON_COMMUNICATION_COMMUNICATIONCOMMON_H_

#include <vector>
#include <sstream>
#include <memory>
#include <string>
#include <utility>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/streambuf.hpp>

#include "Singleton.h"
#include "Logger.h"
#include <boost/asio/buffers_iterator.hpp>


namespace Fatracing {
namespace Communication {

//! Тип буфера
typedef std::vector<uint8_t> Buffer;


struct Message
{
    //! Command ID
    int16_t MessageId = 0;
    
    // Data
    Buffer data;
};



//! Структура которая может парситься из вектора байтов
//! И также сериализовываться
struct SerializableStructure {
	//! Виртуальный деструктор
	virtual ~SerializableStructure() = default;

	//! Распарсить сообщение из буфера
	virtual bool Parse(const Buffer& aBytes) = 0;
	//! Преобразовать сообщение в байты
	virtual void ToBytes(Buffer& aBytes) = 0;

	//! Получить размер сериализованной структуры в байтах
	virtual size_t GetSize() = 0;
};

#pragma region Сетевой порядок байт
enum class EndiannessEnum
{
	LittleEndian,
	BigEndian
};

//! Получить Endianness на данной машине
EndiannessEnum GetEndianness();
//! Порядок байт хоста в сетевой
uint64_t htonll(uint64_t src);
//! Сетевой порядок байт в порядок хоста
uint64_t ntohll(uint64_t input);
//! Сетевой порядок байт в порядок хоста
int64_t ntohll(int64_t input);


std::vector<uint8_t> Int16ToBytes(int16_t int16, EndiannessEnum aEndianness=EndiannessEnum::BigEndian);
std::vector<uint8_t> Int32ToBytes(int32_t int32, EndiannessEnum aEndianness=EndiannessEnum::BigEndian);
std::vector<uint8_t> Int64ToBytes(int64_t int64, EndiannessEnum aEndianness=EndiannessEnum::BigEndian);

std::vector<uint8_t> DoubleToBytes(double paramDouble);
std::vector<uint8_t> FloatToBytes(float paramDouble);


//! Преобразовать в сетевой порядок байт
template<typename T1> 
T1 ToNetwork(T1 aParameter);

template<>
inline int64_t ToNetwork<int64_t>(int64_t aParameter)	
{
	return htonll(aParameter);
}
template<>
inline uint64_t ToNetwork<uint64_t>(uint64_t aParameter)
{
	return htonll(aParameter);
}
template<>
inline int32_t ToNetwork<int32_t>(int32_t aParameter)	
{
	return htonl(aParameter);
}
template<>
inline uint32_t ToNetwork<uint32_t>(uint32_t aParameter)	
{
	return htonl(aParameter);
}
template<>
inline int16_t ToNetwork<int16_t>(int16_t aParameter)	
{
	return htons(aParameter);
}
template<>
inline uint16_t ToNetwork<uint16_t>(uint16_t aParameter)	
{
	return htons(aParameter);
}

//! Преобразовать в порядок байт данной машины
template<typename T1> 
T1 ToHost(T1 aParameter);

template<>
inline int64_t ToHost<int64_t>(int64_t aParameter)	
{
	return ntohll(aParameter);
}
template<>
inline uint64_t ToHost<uint64_t>(uint64_t aParameter)	
{
	return ntohll(aParameter);
}
template<>
inline int32_t ToHost<int32_t>(int32_t aParameter)	
{
	return ntohl(aParameter);
}
template<>
inline uint32_t ToHost<uint32_t>(uint32_t aParameter)	
{
	return ntohl(aParameter);
}
template<>
inline int16_t ToHost<int16_t>(int16_t aParameter)	
{
	return ntohs(aParameter);
}
template<>
inline uint16_t ToHost<uint16_t>(uint16_t aParameter)	
{
	return ntohs(aParameter);
}

#pragma endregion

//! Тип порта сокета
typedef uint16_t Port;
//! Тип работы для io_service
typedef boost::shared_ptr<boost::asio::io_service::work> WorkPointer;

#pragma region Boost.Archive
//! Сериализация структуры в строку с помощью Boost.Archive
template <typename T>
bool ToString(T& serializableStruct, std::string& s)
{
    try
    {
        std::ostringstream oss;
        boost::archive::binary_oarchive oa(oss);
        oa << serializableStruct;
        s = oss.str();
        return true;
    }
    catch (boost::archive::archive_exception &e) // исключение предпочтительно перехватывать по ссылке https://www.viva64.com/ru/w/V746/
    {
        LOGGER_LOG(PriorityEnum::Error, e.what());
        return false;
    }
}

template <typename T>
bool FromString(std::string& s, T& serializableStruct)
{
    try
    {
        std::stringstream ss(s);
        boost::archive::binary_iarchive ia(ss);
        ia >> serializableStruct;
        return true;
    }
    catch (boost::archive::archive_exception &e)
    {
        LOGGER_LOG(PriorityEnum::Error, e.what());
        return false;
    }
}
#pragma endregion

#pragma region Конвертация
//!
std::string BufferToReadable(Buffer& buffer);
//!
std::string StreambufToReadable(boost::asio::streambuf& b);
//! Конвертировать бустовый стримбуф в Buffer
//! @remarks не забудьте освободить память
Buffer* ToBuffer(boost::asio::streambuf* in);
//! Конвертировать строку в Buffer
//! @remarks не забудьте освободить память
Buffer* ToBuffer(const std::string& s);
//! Конвертировать структуру в Buffer
//! @remarks не забудьте освободить память
template <typename T>
Buffer* ToBuffer(T s)
{
    auto i = reinterpret_cast<unsigned char*>(&s);
    return new Buffer(i, i + sizeof(T));
}
#pragma endregion 


void AppendToBuffer(Buffer& aDestination, const Buffer& aToAppend);

template<size_t N>
void AppendToBuffer(Buffer& aDestination, std::array<char, N> aArray)
{
	aDestination.insert(aDestination.end(), aArray.begin(), aArray.end());
}


std::string BufferToReadable(const Buffer& buffer);

#pragma region Extract

template <typename T>
T Extract(const Buffer& aBytes, size_t& aIndex) { return {}; }

template <>
inline float Extract(const Buffer& aBytes, size_t& aIndex)
{	
	auto ret = Utils::Int32ToFloat(
		ToHost(Utils::BytesToInt32(aBytes[aIndex], aBytes[aIndex + 1], aBytes[aIndex + 2], aBytes[aIndex + 3])));
	aIndex += sizeof(int32_t);
	return ret;
}
template <>
inline int32_t Extract(const Buffer& aBytes, size_t& aIndex)
{
	auto ret = ToHost(Utils::BytesToInt32(aBytes[aIndex], aBytes[aIndex + 1], aBytes[aIndex + 2], aBytes[aIndex + 3]));
	aIndex += sizeof(int32_t);
	return ret;
}
template <>
inline uint32_t Extract(const Buffer& aBytes, size_t& aIndex)
{
	auto ret = ToHost(Utils::BytesToUInt32(aBytes[aIndex], aBytes[aIndex + 1], aBytes[aIndex + 2], aBytes[aIndex + 3]));
	aIndex += sizeof(uint32_t);
	return ret;
}
template <>
inline int16_t Extract(const Buffer& aBytes, size_t& aIndex)
{
	auto ret = ToHost(Utils::BytesToInt16(aBytes[aIndex], aBytes[aIndex + 1]));
	aIndex += sizeof(int16_t);
	return ret;
}
template <>
inline uint16_t Extract(const Buffer& aBytes, size_t& aIndex)
{
	auto ret = ToHost(Utils::BytesToUInt16(aBytes[aIndex], aBytes[aIndex + 1]));
	aIndex += sizeof(uint16_t);
	return ret;
}
template <>
inline int8_t Extract(const Buffer& aBytes, size_t& aIndex)
{
	auto ret = aBytes[aIndex];
	aIndex += sizeof(int8_t);
	return ret;
}
template <>
inline uint8_t Extract(const Buffer& aBytes, size_t& aIndex)
{
	auto ret = aBytes[aIndex];
	aIndex += sizeof(uint8_t);
	return ret;
}

template <typename T>
void Extract(const Buffer& aBytes, size_t& aIndex, T& aOut) {
#ifdef _WIN32
	return {};
#else
	return;
#endif
}

template <>
inline void Extract(const Buffer& aBytes, size_t& aIndex, uint64_t& aOut)
{
	aOut = ToHost(Utils::BytesToUInt64(aBytes[aIndex], aBytes[aIndex + 1], aBytes[aIndex + 2], aBytes[aIndex + 3], aBytes[aIndex + 4], aBytes[aIndex + 5], aBytes[aIndex + 6], aBytes[aIndex + 7]));
	aIndex += sizeof(uint64_t);
}
template <>
inline void Extract(const Buffer& aBytes, size_t& aIndex, int32_t& aOut)
{
	aOut =ToHost(Utils::BytesToInt32(aBytes[aIndex], aBytes[aIndex + 1], aBytes[aIndex + 2], aBytes[aIndex + 3]));
	aIndex += sizeof(int32_t);
}
template <>
inline void Extract(const Buffer& aBytes, size_t& aIndex, uint32_t& aOut)
{
	aOut = ToHost(Utils::BytesToUInt32(aBytes[aIndex], aBytes[aIndex + 1], aBytes[aIndex + 2], aBytes[aIndex + 3]));
	aIndex += sizeof(uint32_t);
}

template <>
inline void Extract(const Buffer& aBytes, size_t& aIndex, int16_t& aOut)
{
	aOut = ToHost(Utils::BytesToInt16(aBytes[aIndex], aBytes[aIndex + 1]));
	aIndex += sizeof(int16_t);
}
template <>
inline void Extract(const Buffer& aBytes, size_t& aIndex, uint16_t& aOut)
{
	aOut = ToHost(Utils::BytesToUInt16(aBytes[aIndex], aBytes[aIndex + 1]));
	aIndex += sizeof(int16_t);
}
template <>
inline void Extract(const Buffer& aBytes, size_t& aIndex, int8_t& aOut)
{
	aOut = aBytes[aIndex];
	aIndex += sizeof(int8_t);
}
template <>
inline void Extract(const Buffer& aBytes, size_t& aIndex, uint8_t& aOut)
{
	aOut = aBytes[aIndex];
	aIndex += sizeof(uint8_t);
}
//template <>
//inline void Extract(const Buffer& aBytes, size_t& aIndex, bool& aOut)
//{	
//	if (aBytes[aIndex] == 0)
//		aOut = false;
//	else
//		aOut = true;	
//	aIndex += sizeof(bool);
//}
template <>
inline void Extract(const Buffer& aBytes, size_t& aIndex, float& aOut)
{
	aOut = Utils::Int32ToFloat(ToHost(Utils::BytesToInt32(aBytes[aIndex], aBytes[aIndex + 1], aBytes[aIndex + 2], aBytes[aIndex + 3])));
	aIndex += sizeof(int32_t);
}
//template <>
//inline void Extract(const Buffer& aBytes, size_t& aIndex, std::string& aOut)
//{
//	std::copy(aBytes.begin() + aIndex, aBytes.begin() + aIndex + aOut.size(), aOut.data());
//	aIndex += aOut.size();	
//}
template<size_t N>
void Extract(const Buffer& aBytes, size_t& aIndex, std::array<char,N>& aOut)
{
	std::copy(aBytes.begin() + aIndex, aBytes.begin() + aIndex+ aOut.size(), aOut.data());
	aIndex += aOut.size();
}
#pragma endregion 

#pragma region Pack
template<typename T>
void Pack(Buffer& aBytes, T aValue) {
#ifdef _WIN32
	return {};
#else
	return;
#endif
}

template<>
inline void Pack(Buffer& aBytes, float aValue)
{
AppendToBuffer(aBytes, Int32ToBytes(ToNetwork(Utils::FloatToInt32(aValue))));
}
template<>
inline void Pack(Buffer& aBytes, int32_t aValue)
{
AppendToBuffer(aBytes, Int32ToBytes(ToNetwork(aValue)));
}
template<>
inline void Pack(Buffer& aBytes, uint32_t aValue)
{
AppendToBuffer(aBytes, Int32ToBytes(ToNetwork(aValue)));
}
template<>
inline void Pack(Buffer& aBytes, int16_t aValue)
{
AppendToBuffer(aBytes, Int16ToBytes(ToNetwork(aValue)));
}
template<>
inline void Pack(Buffer& aBytes, uint16_t aValue)
{
AppendToBuffer(aBytes, Int16ToBytes(ToNetwork(aValue)));
}

#pragma endregion

} // namespace Communication
} // namespace AFA

#endif // AFA41_10_MAIN_SUBPRJ_COMMON_COMMUNICATION_COMMUNICATIONCOMMON_H_
