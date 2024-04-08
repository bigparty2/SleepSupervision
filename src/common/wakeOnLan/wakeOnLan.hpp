#ifndef WAKEONLAN_HPP
#define WAKEONLAN_HPP

#include <string>
#include <stdexcept>
#include "../socket/socket.hpp"
#include "../types.hpp"
#include "../string/string.hpp"

namespace ss
{
    namespace network
    {
        //Wake-on-LAN (WoL): acordar dispositivos que suportam a tecnologia
        class wakeOnLan
        {
            public:

            //Classe para construção do pacote mágico
            class magicPacket
            {
                //instancia de um magic packet 
                std::string mp;

                //tamanho de um magic packet
                static const unsigned char MAGIC_PACKET_SIZE = 102;

                public:
    
                //constructor recebendo MAC address do computador
                magicPacket(const std::string &macAddr);

                //Constroe o pacote magico a partir de um endereço mac de forma estática
                static std::string Build(const std::string &macAddr);

                //Transformação do pacote magico para string
                static std::string ToString(const std::string &mp);
                std::string ToString();

                //Retorna o pacote mágico para transmissão da mensagem
                const char* Get();

                //Retorna o tamanho do pacote mágico
                static byte Length();
            };

            private:

            //Porta padrão do WakeOnLan
            static const unsigned short DEFAULT_PORT = 9;
            
            public:

            //Acorda um computador com base em seu endereço MAC
            static void Awake(const std::string &macAddress, const byte &port = DEFAULT_PORT);
        };
    }
}

#endif //WAKEONLAN_HPP