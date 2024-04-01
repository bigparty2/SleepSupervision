#ifndef STRING_HPP
#define STRING_CPP

#include <string>
#include <sstream>
#include <vector>
#include <iostream>

namespace ss
{
    class string
    {
        public:

        static std::string Repeat(const unsigned int &n, const std::string &strToRepeat);

        static std::string ToCenter(const std::string &str, unsigned int length);

        static std::wstring ToCenter(const std::wstring &str, unsigned int length);

        static std::vector<std::string> Split(const std::string &str, char delimiter);
    };
}

#endif //STRING_CPP