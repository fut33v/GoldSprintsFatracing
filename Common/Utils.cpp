// Copyright 2017 Pelengator

#include <ctime>
#include "Utils.h"
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

namespace VMM
{
namespace Utils
{
tm TimeToTimeT(const std::chrono::system_clock::time_point& time)
{
    tm t;
    time_t tt = std::chrono::system_clock::to_time_t(time);
#ifdef _WIN32
    localtime_s(&t, &tt);
#elif __linux__
    tm* pointer;
    pointer = localtime(&tt);
    t = *pointer;
#endif
    return t;
}

tm GetNowTm()
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	return TimeToTimeT(now);
}

std::chrono::system_clock::time_point TmToTimePoint(tm& aTm)
{
	auto t = mktime(&aTm);
	return std::chrono::system_clock::from_time_t(t);
}

std::string Format(const std::chrono::system_clock::time_point& time)
{
    auto tp = time.time_since_epoch();
    tp -= std::chrono::duration_cast<std::chrono::seconds>(tp);

    tm t = TimeToTimeT(time);
    return Format("[%04u-%02u-%02u %02u:%02u:%02u.%03u] ", t.tm_year + 1900,
                  t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,
                  static_cast<unsigned int>(tp / std::chrono::milliseconds(1)));
}

std::string FormatTimeShort(const std::chrono::system_clock::time_point& time)
{
	auto tp = time.time_since_epoch();
	tp -= std::chrono::duration_cast<std::chrono::seconds>(tp);
	tm t = TimeToTimeT(time);
	return Format("[%02u:%02u:%02u.%03u] ",
	              t.tm_hour, t.tm_min, t.tm_sec,
	              static_cast<unsigned int>(tp / std::chrono::milliseconds(1)));
}

std::string FormatFileName(const std::chrono::system_clock::time_point& time)
{
    auto tp = time.time_since_epoch();
    tp -= std::chrono::duration_cast<std::chrono::seconds>(tp);

    tm t = TimeToTimeT(time);
    return Format("%04u-%02u-%02u_%02u-%02u-%02u.%03u", t.tm_year + 1900,
        t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,
        static_cast<unsigned int>(tp / std::chrono::milliseconds(1)));
}

std::string FormatFileNameYearDayMonth(std::chrono::system_clock::time_point time)
{
	auto tp = time.time_since_epoch();
	tp -= std::chrono::duration_cast<std::chrono::seconds>(tp);

    tm t = TimeToTimeT(time);

	return Format("%04u_%02u_%02u", t.tm_year + 1900, t.tm_mday, t.tm_mon + 1);
}

std::string FormatFileNameYearMonthDayHourSecond(std::chrono::system_clock::time_point time)
{
	auto tp = time.time_since_epoch();
	tp -= std::chrono::duration_cast<std::chrono::seconds>(tp);

    tm t = TimeToTimeT(time);

	return Format("%04u_%02u_%02u__%02u_%02u_%02u", t.tm_year + 1900, t.tm_mday, t.tm_mon + 1, t.tm_hour, t.tm_min, t.tm_sec);
}

static std::unordered_map<std::string, ColorType> ColorTypes = {    
    {"Monochrome", ColorType::Monochrome}    
};

ColorType ColorTypeFromStr(const std::string& str)
{
    auto itr = ColorTypes.find(str);
    if (itr != ColorTypes.end())
    {
        return itr->second;
    }
    return ColorType::Monochrome;
}

std::string ToString(double n, int precision, std::string locale)
{
    std::stringstream stream;
//    stream.imbue(std::locale(locale));
    stream << std::fixed << std::setprecision(precision) << n;
    return stream.str();
}

void Split(const std::string& aSource, std::vector<std::string>& aDest, const char aDelimiter, bool aTrim)
{    
    if (!aSource.empty())
    {
        std::stringstream strStream(aSource);
        std::string item;
        while (std::getline(strStream, item, aDelimiter))
        {
            aDest.push_back(aTrim ? Trim(item) : item);
        }
    }
}


int32_t FloatToInt32(float aFloat)
{
	return *reinterpret_cast<int32_t*>(&aFloat);
}

int64_t DoubleToInt64(double aDouble)
{
	return *reinterpret_cast<int64_t*>(&aDouble);
}



int32_t BytesToInt32(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4)
{
	return b1 << 24 |
		b2 << 16 |
		b3 << 8 |
		b4;
}

uint32_t BytesToUInt32(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4) {
	return b1 << 24 |
		b2 << 16 |
		b3 << 8 |
		b4;
}


int64_t BytesToInt64(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7, uint8_t b8)
{
	return
		static_cast<int64_t>(b1) << 56 |
		static_cast<int64_t>(b2) << 48 |
		static_cast<int64_t>(b3) << 40 |
		static_cast<int64_t>(b4) << 32 |
		static_cast<int64_t>(b5) << 24 |
		static_cast<int64_t>(b6) << 16 |
		static_cast<int64_t>(b7) << 8  |
		b8;
}

int64_t BytesToUInt64(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7, uint8_t b8)
{
	return
		static_cast<uint64_t>(b1) << 56 |
		static_cast<uint64_t>(b2) << 48 |
		static_cast<uint64_t>(b3) << 40 |
		static_cast<uint64_t>(b4) << 32 |
		static_cast<uint64_t>(b5) << 24 |
		static_cast<uint64_t>(b6) << 16 |
		static_cast<uint64_t>(b7) << 8  |
		b8;
}

int16_t BytesToInt16(uint8_t b1, uint8_t b2)
{
	return static_cast<int16_t>(b1 << 8 | b2);
}

uint16_t BytesToUInt16(uint8_t b1, uint8_t b2) {
	return static_cast<uint16_t>(b1 << 8 | b2);
}

float Int32ToFloat(int32_t int32)
{
	return *reinterpret_cast<float*>(&int32);
}

float Uint32ToFloat(uint32_t uint32) {
	return *reinterpret_cast<float*>(&uint32);
}


double Int64ToDouble(int64_t int64)
{
	return *reinterpret_cast<double*>(&int64);
}

double Uint64ToDouble(uint64_t uint64)
{
	return *reinterpret_cast<double*>(&uint64);
}

using namespace boost::filesystem;
using namespace boost::system;
uint64_t GetFolderSize(path aPath)
{
    uint64_t size = 0;
    error_code ec;
    for (recursive_directory_iterator it(aPath); it != recursive_directory_iterator(); it.increment(ec))
    {
        if (!is_directory(*it))
            size += file_size(*it);
    }
    return size;
}

uint64_t GetFileCount(path aPath, const std::string& aExt)
{
    uint64_t count = 0;
    error_code ec;
    for (recursive_directory_iterator it(aPath); it != recursive_directory_iterator(); it.increment(ec))
    {
        if (is_regular_file(*it))
        {
            if ((*it).path().extension().string() == aExt)
            {
                count++;
            }
        }
    }
    return count;
}

void CheckPath(std::string& aPath)
{

#ifdef _WIN32
   char delimiter = '\\' ;
#elif __linux__
   char delimiter = '/' ;
#endif

    if (!aPath.empty() && *aPath.rbegin() != delimiter)
    {
        aPath += delimiter;
    }
}

std::string CheckPathCopy(const std::string& aPath)
{
    std::string result = aPath;
    CheckPath(result);
    return result;
}

path GetFolderPath(const std::string& aPath)
{
    path mainFolderPath(aPath);
    error_code error;

    if (!exists(mainFolderPath, error))
    {
        return mainFolderPath;
    }

    mainFolderPath.remove_trailing_separator();

    for (int i = 1;;i++)
    {
        path temp = mainFolderPath;
        temp += "_" + std::to_string(i);
        if (!exists(temp, error))
        {
            return temp;
        }
    }
}

std::list<path> GetSubFoldersSortedByName(path aPath)
{
    std::list<path> subfolders;
    if (exists(aPath) && is_directory(aPath))
    {
        for (auto& x : directory_iterator(aPath))
        {
            if (is_directory(x))
                subfolders.emplace_back(x);
        }
        subfolders.sort();
    }
    return subfolders;
}


std::string GetFileNameWithExtension(std::string aFilePath) {
	std::vector<std::string> tokens;
	Utils::Split(aFilePath, tokens, '\\');
	return tokens.empty() ?  aFilePath :  tokens.back();
}

int GetNumberOfDays(int month, int year) {
	// високосный год
	if (month == 2) {
		if ((year % 400 == 0) || (year % 4 == 0 && year % 100 != 0))
			return 29;
		return 28;
	}
	if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8
		|| month == 10 || month == 12)
		return 31;
	return 30;
}

std::string GetFileContents(const char *filename) {
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (in) {
        return(std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
    }
    throw(errno);
}

bool SaveContentsToFile(const char *filename, std::string contents) {
    try {
        std::ofstream out(filename);
        out << contents;
        out.close();
        return true;
    } catch (std::exception& e) {
        return false;
    }
}


} // namespace Utils
} // namespace VMM
