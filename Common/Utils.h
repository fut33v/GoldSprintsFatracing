// Copyright 2018

#ifndef SUBPRJ_COMMON_BASE_UTILS_H_
#define SUBPRJ_COMMON_BASE_UTILS_H_

#include <memory>
#include <chrono>
#include <string>
#include <unordered_map>
#include <stdarg.h>
#include <map>
#include <iomanip>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/path.hpp>
#include <sstream>
#include <list>

#include "CameraAPI/ICameraAPI.h"
#include <chrono>

namespace VMM
{
namespace Utils
{
template <typename... Args>
std::string Format(const char* format, Args&&... vs)
{
    char b;
    unsigned int required = std::snprintf(&b, 0, format, vs...) + 1;

    std::unique_ptr<char[]> bytes = std::unique_ptr<char[]>(new char[required]);
    std::snprintf(bytes.get(), required, format, vs...);

    return std::string(bytes.get());
}

tm TimeToTimeT(const std::chrono::system_clock::time_point& time);
tm GetNowTm();
std::chrono::system_clock::time_point TmToTimePoint(tm& aTm);

std::string Format(const std::chrono::system_clock::time_point& time);

std::string FormatTimeShort(const std::chrono::system_clock::time_point& time);

std::string FormatFileName(const std::chrono::system_clock::time_point& time);
std::string FormatFileNameYearDayMonth(std::chrono::system_clock::time_point time);
std::string FormatFileNameYearMonthDayHourSecond(std::chrono::system_clock::time_point time);

ColorType ColorTypeFromStr(const std::string& str);




//! Сгенерировать id для мапы
template <typename K, typename V>
K FindId(std::map<K, V> m)
{
    K id = 0;
    for (id = 1; id <= INT_MAX; ++id)
    {
        if (m.find(id) == m.end())
        {
            break;
        }
    }
    return id;
}


std::string ToString(double n, int precision = 2, std::string locale="en_US.UTF8");

template <typename T>
bool FromString(const std::string& s, T& n)
{
    try {
        n = boost::lexical_cast<T>(s);
    } 
    catch (boost::bad_lexical_cast& /*e*/)
    {
        return false;
    }
    return true;
}


inline std::string GetBeforeNewline(const std::string& s)
{
    size_t index = s.size()-1;
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == '\n')
        {
            index = i;
        }
    }
    return s.substr(0, index);
}

// trim from end of string (right)
inline std::string& Rtrim(std::string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from beginning of string (left)
inline std::string& Ltrim(std::string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from both ends of string (left & right)
inline std::string& Trim(std::string& s, const char* t = " \t\n\r\f\v")
{
    return Ltrim(Rtrim(s, t), t);
}

void Split(const std::string& aSource, std::vector<std::string>& aDest, const char aDelimiter, bool aTrim = false);

int32_t FloatToInt32(float aFloat);
int64_t DoubleToInt64(double aDouble);

//! Big-endian
int64_t BytesToInt64(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7, uint8_t b8);
//! Big-endian
int64_t BytesToUInt64(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7, uint8_t b8);
//! Big-endian
int32_t BytesToInt32(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4);
//! Big-endian
uint32_t BytesToUInt32(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4);
//! Big-endian
int16_t BytesToInt16(uint8_t b1, uint8_t b2);
//! Big-endian
uint16_t BytesToUInt16(uint8_t b1, uint8_t b2);

float Int32ToFloat(int32_t int32);
float Uint32ToFloat(uint32_t uint32);
double Int64ToDouble(int64_t int64);
double Uint64ToDouble(uint64_t uint64);


void CheckPath(std::string& aPath);
std::string CheckPathCopy(const std::string& aPath);

boost::filesystem::path GetFolderPath(const std::string& aPath);

uint64_t GetFolderSize(boost::filesystem::path aPath);

std::list<boost::filesystem::path> GetSubFoldersSortedByName(boost::filesystem::path aPath);

uint64_t GetFileCount(boost::filesystem::path aPath, const std::string& aExt);

std::string GetFileNameWithExtension(std::string aFilePath);

template <std::size_t N>
void StringToArray(std::string aString, std::array<char, N>& aArray) {
	aArray = std::array<char, N>();
	if (aString.size() > aArray.size())
		return;
	std::copy(aString.begin(), aString.begin() + aString.size(), aArray.data());
}

template <std::size_t N>
std::string ArrayToString(std::array<char, N> aArray) {
	return std::string (std::begin(aArray), std::end(aArray));
}


inline float GetRandomFloat(float aMin, float aMax)
{
	return aMin + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (aMax - aMin)));
}

inline int GetRandomInt(int aMin, int aMax)
{
	return  aMin + (rand() % static_cast<int>(aMax - aMin + 1));
}

//! Полчить количество дней в месяце
int GetNumberOfDays(int month, int year);

//! для конвертации float в массив байт
union Float
{
	float m_float;
	uint8_t m_bytes[sizeof(float)];
};

//! для конвертации float в массив байт
union Double
{
	double m_float;
	uint8_t m_bytes[sizeof(double)];
};


std::string GetFileContents(const char *filename);
bool SaveContentsToFile(const char *filename, std::string contents);

} // namespace Utils
} // namespace VMM

#endif // SUBPRJ_COMMON_BASE_UTILS_H_
