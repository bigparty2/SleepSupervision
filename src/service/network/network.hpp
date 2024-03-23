#ifndef NETWORK_HPP
#define NETWORK_HPP

#include "../../common/common.hpp"

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
                std::string _magicPacket;

                //tamanho de um magic packet
                const unsigned char MAGIC_PACKET_SIZE = 102;

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
                static unsigned char Length();
            };

            private:

            //Porta padrão do WakeOnLan
            static const unsigned short DEFAULT_PORT = 9;
            
            public:

            //Acorda um computador com base em seu endereço MAC
            static void Awake(const std::string &macAddress, const unsigned short &port = DEFAULT_PORT);
        };
    
        //Socket
        class socket
        {
        };

        //Estrutura para tratamento de endereço MAC
        class MAC
        {
        public:

            //constructor
            MAC(){};
            MAC(std::string addr);
            MAC(const unsigned char* addr);

            //operadores
            MAC& operator=(const std::string&);
            MAC& operator=(const MAC&);
            bool operator==(const MAC&);

            //Retorna o endereço MAC como string
            std::string ToString();
            std::string ToWString();

            //Retorna o endereço MAC como ponteiro
            byte* Get();

        private:

            //Atribui o MAC recebido ao arrey de byte
            void StrToArray(std::string mac);

            //Armazena um endereço MAC
            byte _address[6];
        };

        //Estrutura para tratamento de endereço IPV4
        class IPV4
        {
        public:

            //constructor
            IPV4(){}
            IPV4(uint32_t addr);
            IPV4(std::string addr);

            //operador
            IPV4& operator=(const std::string&);
            IPV4& operator=(const uint32_t&);
            bool operator==(const IPV4&);

            //Retorna o IP como string
            std::string ToString();
            std::wstring ToWString();

            //Retorna o endereço IP como int32
            uint32_t Get();

        private:

            //de Int32 para ByteArr
            void IntToByteArr(uint32_t addr);

            //de ByteArr para Int32
            uint32_t ByteArrToInt();

            //Armazenamento do endereço (network byte order)
            uint32_t _address = 0;
        };

    }
}

#endif //NETWORK_HPP