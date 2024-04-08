#ifndef IPV4_HPP
#define IPV4_HPP

#include <string>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <ifaddrs.h> 
#include <arpa/inet.h>
#include "../types.hpp"

namespace ss
{
    namespace network
    {
        //Estrutura para tratamento de endereço IPV4
        class IPV4
        {
        public:

            //constructor
            IPV4(){}
            IPV4(uint32_t addr);
            IPV4(const std::string& addr);

            //operador
            IPV4& operator=(const std::string&);
            IPV4& operator=(const uint32_t&);
            bool operator==(const IPV4&);

            //Retorna o IP como string
            std::string ToString();
            std::wstring ToWString();

            //Retorna o endereço IP como int32
            uint32_t Get();

            //Pega o endereco IPV4 local 
            void GetIPV4(std::string interface);

        private:

            //de Int32 para ByteArr
            void IntToByteArr(uint32_t addr);

            //de ByteArr para Int32
            uint32_t ByteArrToInt();

            // Validação de IPV4 como string
            void IPV4Validation(const std::string& addr);

            //Armazenamento do endereço (network byte order)
            uint32_t _address = 0;
        };
    }
}

#endif //IPV4_HPP