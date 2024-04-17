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
        /**
         * @brief Classe para acordar dispositivos que suportam a tecnologia Wake-on-LAN (WoL).
         * 
         * A classe `wakeOnLan` permite acordar dispositivos que suportam a tecnologia Wake-on-LAN (WoL).
         * O WoL é uma tecnologia que permite ligar dispositivos remotamente através da rede, enviando um pacote mágico (magic packet) para o endereço MAC do dispositivo.
         * 
         * A classe possui uma classe interna `magicPacket` que é responsável por construir o pacote mágico.
         * O pacote mágico é um pacote de tamanho fixo (102 bytes) que contém informações necessárias para acordar o dispositivo.
         * 
         * A classe `wakeOnLan` também possui um método estático `Awake` que permite acordar um computador com base em seu endereço MAC.
         * O método `Awake` envia o pacote mágico para o endereço MAC especificado, utilizando a porta padrão do WoL (porta 9).
         */
        class wakeOnLan
        {
            public:

            /**
             * @brief Classe interna para construção do pacote mágico.
             * 
             * A classe `magicPacket` é responsável por construir o pacote mágico utilizado para acordar o dispositivo.
             * O pacote mágico é um pacote de tamanho fixo (102 bytes) que contém informações necessárias para acordar o dispositivo.
             */
            class magicPacket
            {
                //instancia de um magic packet 
                std::string mp;

                //tamanho de um magic packet
                static const unsigned char MAGIC_PACKET_SIZE = 102;

                public:
    
                /**
                 * @brief Construtor da classe `magicPacket`.
                 * 
                 * @param macAddr O endereço MAC do computador.
                 */
                magicPacket(const std::string &macAddr);

                /**
                 * @brief Constrói o pacote mágico a partir de um endereço MAC de forma estática.
                 * 
                 * @param macAddr O endereço MAC do computador.
                 * @return O pacote mágico construído.
                 */
                static std::string Build(const std::string &macAddr);

                /**
                 * @brief Converte o pacote mágico para uma string.
                 * 
                 * @param mp O pacote mágico.
                 * @return O pacote mágico convertido para uma string.
                 */
                static std::string ToString(const std::string &mp);

                /**
                 * @brief Converte o pacote mágico para uma string.
                 * 
                 * @return O pacote mágico convertido para uma string.
                 */
                std::string ToString();

                /**
                 * @brief Retorna o pacote mágico para transmissão da mensagem.
                 * 
                 * @return O pacote mágico.
                 */
                const char* Get();

                /**
                 * @brief Retorna o tamanho do pacote mágico.
                 * 
                 * @return O tamanho do pacote mágico.
                 */
                static byte Length();
            };

            private:

            //Porta padrão do WakeOnLan
            static const unsigned short DEFAULT_PORT = 9;
            
            public:

            /**
             * @brief Acorda um computador com base em seu endereço MAC.
             * 
             * @param macAddress O endereço MAC do computador.
             * @param port A porta utilizada para enviar o pacote mágico (opcional, padrão é a porta 9).
             */
            static void Awake(const std::string &macAddress, const byte &port = DEFAULT_PORT);
        };
    }
}

#endif //WAKEONLAN_HPP