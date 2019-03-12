#ifndef SUBPRJ_CONVERT_COMMON_H_
#define SUBPRJ_CONVERT_COMMON_H_

#include <boost/locale.hpp>
#include <iomanip>

namespace VMM
{
namespace Utils
{
//! Конвертируем виндово закодированную русню в юникодик
inline std::string Win1251ToUTF8(const std::string& s)
{
    if (s.empty())
    {
        return s;
    }
    return boost::locale::conv::to_utf<char>(s, "Windows-1251");
}

template <typename T>
std::string IntToHex(T i)
{
    std::stringstream sstream;
    sstream << std::hex << i;
    std::string result = sstream.str();
    return result;
}


inline std::string GetStringTail(std::string const& source, size_t const length)
{
    if (length >= source.size()) { return source; }
    return source.substr(source.size() - length);
} 


inline std::string CharToHex(char i)
{
    std::stringstream sstream;
    sstream << std::hex << static_cast<int>(i);
    std::string result = sstream.str();
    result = GetStringTail(result, 2);
    return result;
}
}
}

#endif // SUBPRJ_AFA_COMMON_H_
