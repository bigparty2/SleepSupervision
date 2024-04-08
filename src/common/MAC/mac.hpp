#ifndef MAC_HPP
#define MAC_HPP

#include <string>
#include <ostream>
#include <cstring>
#include <stdexcept>
#include <ifaddrs.h> 
#include <netpacket/packet.h>
#include "../types.hpp"
#include "../logger/logger.hpp"

namespace ss
{
    namespace network
    {
        //Estrutura para tratamento de endereço MAC
        class MAC
        {
        public:

            //constructor
            MAC(){};
            MAC(std::string addr);
            MAC(byte* addr);

            //operadores
            MAC& operator=(const std::string&);
            MAC& operator=(const MAC&);
            bool operator==(const MAC&);

            //Retorna o endereço MAC como string
            std::string ToString();
            static std::string ToString(byte* addr);
            std::string ToWString();

            //Retorna o endereço MAC como ponteiro
            byte* Get();

            //Pega o endereco MAC da maquina local
            void GetMACAddrr(std::string interface);

        private:

            //Definie o endereço MAC
            void Set(const byte* addr);

            //Atribui o MAC recebido ao arrey de byte
            void StrToArray(std::string mac);

            //Armazena um endereço MAC
            byte address[6] = {};
            // byte* address;
        };
    }
}

#endif //MAC_HPP