#include "string.hpp"

using namespace ss;

std::string string::Repeat(const unsigned int &n, const std::string &strToRepeat)
{
    std::ostringstream oss;

    for(int i = 0; i < n; i++)
        oss << strToRepeat;

    return oss.str();
}

std::string string::ToCenter(const std::string &str, unsigned int length)
{
    auto strLen = str.length();

    if(strLen >= length)
        return str;

    uint32_t pad = (length - strLen)/2;
    bool isOdd = ((length - strLen) % 2) != 0;

    return std::string(pad, ' ') + str + std::string(isOdd ? pad + 1 : pad, ' ');
}

std::wstring string::ToCenter(const std::wstring &str, unsigned int length)
{
    auto strLen = str.length();

    if(strLen >= length)
        return str;

    uint32_t pad = (length - strLen)/2;
    bool isOdd = ((length - strLen) % 2) != 0;

    return std::wstring(pad, ' ') + str + std::wstring(isOdd ? pad + 1 : pad, ' ');
}

std::vector<std::string> string::Split(const std::string& str, char delimiter)
{
    std::stringstream stream(str);
    std::vector<std::string> tokens;
    std::string token;
    
    while (std::getline(stream, token, delimiter)) 
    {
        tokens.push_back(token);
    }

    return tokens;
}
