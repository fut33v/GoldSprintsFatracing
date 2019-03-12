// Copyright 2017 Pelengator


#include <memory>
#include <string>
#include <vector>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/host_name.hpp>

#include "Logger.h"

#include "./CommunicationCommon.h"


namespace VMM
{
namespace Communication
{
EndiannessEnum GetEndianness()
{
	EndiannessEnum typ;
	union
	{
		unsigned long long ull;
		unsigned char c[8];
	} x;
	x.ull = 0x01;
	typ = (x.c[7] == 0x01ULL) ? EndiannessEnum::BigEndian : EndiannessEnum::LittleEndian;
	return typ;
}

int64_t htonll___(uint64_t src)
{
	unsigned char c;
	union
	{
		unsigned long long ull;
		unsigned char c[8];
	} x;

	if (GetEndianness() == EndiannessEnum::BigEndian)
		return src;

	x.ull = src;
	c = x.c[0];
	x.c[0] = x.c[7];
	x.c[7] = c;
	c = x.c[1];
	x.c[1] = x.c[6];
	x.c[6] = c;
	c = x.c[2];
	x.c[2] = x.c[5];
	x.c[5] = c;
	c = x.c[3];
	x.c[3] = x.c[4];
	x.c[4] = c;
	return x.ull;
}

uint64_t ntohll(uint64_t input)
{
	if (GetEndianness() == EndiannessEnum::BigEndian)
	{
		return input;
	}

	uint64_t rval;
	uint8_t* data = reinterpret_cast<uint8_t *>(&rval);

	data[0] = (uint8_t)(input >> 56);
	data[1] = (uint8_t)(input >> 48);
	data[2] = (uint8_t)(input >> 40);
	data[3] = (uint8_t)(input >> 32);
	data[4] = (uint8_t)(input >> 24);
	data[5] = (uint8_t)(input >> 16);
	data[6] = (uint8_t)(input >> 8);
	data[7] = (uint8_t)(input >> 0);

	return rval;
}

int64_t ntohll(int64_t input)
{
	if (GetEndianness() == EndiannessEnum::BigEndian)
	{
		return input;
	}

	int64_t rval;
	uint8_t* data = reinterpret_cast<uint8_t *>(&rval);

	data[0] = (uint8_t)(input >> 56);
	data[1] = (uint8_t)(input >> 48);
	data[2] = (uint8_t)(input >> 40);
	data[3] = (uint8_t)(input >> 32);
	data[4] = (uint8_t)(input >> 24);
	data[5] = (uint8_t)(input >> 16);
	data[6] = (uint8_t)(input >> 8);
	data[7] = (uint8_t)(input >> 0);

	return rval;
}


std::string BufferToReadable(Buffer& buffer)
{
    std::string lineReadable = "";
    for (auto& c : buffer)
    {
        lineReadable += Utils::CharToHex(c);
        lineReadable += " ";
    }
    return lineReadable;
}


std::string StreambufToReadable(boost::asio::streambuf& b)
{
    using boost::asio::buffers_begin;
    auto bufs = b.data();
    std::string charString(buffers_begin(bufs), buffers_begin(bufs) + b.size());
    std::string lineReadable;
    for (auto& c: charString)
    {
        lineReadable += Utils::CharToHex(c);
        lineReadable += " ";
    }
    return lineReadable;
}

Buffer* ToBuffer(boost::asio::streambuf* in)
{
    std::istream istream(in);
    Buffer* b = new Buffer(std::istreambuf_iterator<char>(istream), std::istreambuf_iterator<char>());
    return b;
}

Buffer* ToBuffer(const std::string& s) {
    return new Buffer(s.begin(), s.end());
}

void AppendToBuffer(Communication::Buffer& aDestination, const Communication::Buffer& aToAppend)
{
	aDestination.insert(aDestination.end(), aToAppend.begin(), aToAppend.end());
}

std::string BufferToReadable(const Buffer& buffer)
{
	std::string lineReadable = "";
	for (size_t i = 0; i < buffer.size(); i++)
	{
		lineReadable += Utils::CharToHex((buffer)[i]);
		lineReadable += " ";
	}
	return lineReadable;
}

std::vector<uint8_t> Int16ToBytes(int16_t paramInt, EndiannessEnum aEndianness) {
	std::vector<uint8_t> arrayOfByte(2);
	auto msb = static_cast<uint8_t>(paramInt >> 8);
	auto lsb = static_cast<uint8_t>(paramInt);
	switch(aEndianness) {
	case EndiannessEnum::LittleEndian: {
		arrayOfByte[0] = lsb;
		arrayOfByte[1] = msb;
		break;
	}
	case EndiannessEnum::BigEndian: {
		arrayOfByte[0] = msb;
		arrayOfByte[1] = lsb;
		break;
	}
	}
	return arrayOfByte;
}

std::vector<uint8_t> Int32ToBytes(int32_t paramInt, EndiannessEnum aEndianness)
{
	std::vector<uint8_t> arrayOfByte(4);
	switch (aEndianness) {
	case EndiannessEnum::LittleEndian: {
		for (int i = 0; i < 4; i++) {
			arrayOfByte[i] = static_cast<uint8_t>(paramInt >> i * 8);
		}
		break;
	}
	case EndiannessEnum::BigEndian: {
		for (int i = 0; i < 4; i++) {
			arrayOfByte[3 - i] = static_cast<uint8_t>(paramInt >> i * 8);
		}
		break;
	}
	}
	return arrayOfByte;
}

std::vector<uint8_t> Int64ToBytes(int64_t paramInt, EndiannessEnum aEndianness) {
	std::vector<uint8_t> arrayOfByte(8);
	switch (aEndianness) {
	case EndiannessEnum::LittleEndian: {
		for (int i = 0; i < 8; i++) {
			arrayOfByte[i] = static_cast<uint8_t>(paramInt >> i * 8);
		}
		break;
	}
	case EndiannessEnum::BigEndian: {
		for (int i = 0; i < 8; i++) {
			arrayOfByte[7 - i] = static_cast<uint8_t>(paramInt >> i * 8);
		}
		break;
	}
	}
	return arrayOfByte;
}

std::vector<uint8_t> DoubleToBytes(double paramDouble) {
	return Int64ToBytes(*reinterpret_cast<int64_t*>(&paramDouble));
}

std::vector<uint8_t> FloatToBytes(float paramDouble) {
	return Int32ToBytes(*reinterpret_cast<int32_t*>(&paramDouble));
}

} // namespace Communication
} // namespace FM
