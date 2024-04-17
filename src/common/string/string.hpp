#ifndef STRING_HPP
#define STRING_CPP

#include <string>
#include <sstream>
#include <vector>
#include <iostream>

namespace ss
{
    /**
     * @brief A classe string fornece funções utilitárias para manipulação de strings.
     */
    class string
    {
        public:

        /**
         * @brief Repete uma determinada string um número especificado de vezes.
         * @param n O número de vezes que a string deve ser repetida.
         * @param strToRepeat A string a ser repetida.
         * @return A string repetida.
         */
        static std::string Repeat(const unsigned int &n, const std::string &strToRepeat);

        /**
         * @brief Centers a string within a specified length.
         * @param str The string to be centered.
         * @param length The length of the resulting centered string.
         * @return The centered string.
         */
        static std::string ToCenter(const std::string &str, unsigned int length);

        /**
         * @brief Centers a wide string within a specified length.
         * @param str The wide string to be centered.
         * @param length The length of the resulting centered wide string.
         * @return The centered wide string.
         */
        static std::wstring ToCenter(const std::wstring &str, unsigned int length);

        /**
         * @brief Splits a string into a vector of substrings based on a delimiter.
         * @param str The string to be split.
         * @param delimiter The character used as a delimiter.
         * @return A vector of substrings.
         */
        static std::vector<std::string> Split(const std::string &str, char delimiter);
    };
}

#endif //STRING_CPP