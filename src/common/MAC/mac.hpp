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
        /**
         * @brief Classe que representa um endereço MAC.
         * 
         * A classe MAC é usada para manipular endereços MAC em um formato de string ou array de bytes.
         * Ela fornece métodos para converter entre os formatos de string e array de bytes, bem como para obter
         * o endereço MAC da máquina local.
         */
        class MAC
        {
        public:

            /**
             * @brief Construtor padrão da classe MAC.
             */
            MAC(){};

            /**
             * @brief Construtor da classe MAC que recebe um endereço MAC como string.
             * 
             * @param addr O endereço MAC como string.
             */
            MAC(std::string addr);

            /**
             * @brief Construtor da classe MAC que recebe um endereço MAC como array de bytes.
             * 
             * @param addr O endereço MAC como array de bytes.
             */
            MAC(byte* addr);

            /**
             * @brief Sobrecarga do operador de atribuição para atribuir um endereço MAC como string.
             * 
             * @param addr O endereço MAC como string.
             * @return Uma referência para o objeto MAC atual.
             */
            MAC& operator=(const std::string&);

            /**
             * @brief Sobrecarga do operador de atribuição para atribuir um endereço MAC como objeto MAC.
             * 
             * @param other O objeto MAC contendo o endereço MAC a ser atribuído.
             * @return Uma referência para o objeto MAC atual.
             */
            MAC& operator=(const MAC&);

            /**
             * @brief Sobrecarga do operador de igualdade para comparar dois endereços MAC.
             * 
             * @param other O objeto MAC a ser comparado.
             * @return true se os endereços MAC forem iguais, false caso contrário.
             */
            bool operator==(const MAC&);

            /**
             * @brief Retorna o endereço MAC como string.
             * 
             * @return O endereço MAC como string.
             */
            std::string ToString();

            /**
             * @brief Converte um endereço MAC representado como array de bytes para string.
             * 
             * @param addr O endereço MAC como array de bytes.
             * @return O endereço MAC como string.
             */
            static std::string ToString(byte* addr);

            /**
             * @brief Retorna o endereço MAC como string formatado para uso em wide strings.
             * 
             * @return O endereço MAC como string formatado para uso em wide strings.
             */
            std::string ToWString();

            /**
             * @brief Retorna o endereço MAC como array de bytes.
             * 
             * @return O endereço MAC como array de bytes.
             */
            byte* Get();

            /**
             * @brief Obtém o endereço MAC da máquina local para a interface especificada.
             * 
             * @param interface A interface de rede para a qual o endereço MAC deve ser obtido.
             */
            void GetMACAddrr(std::string interface);

        private:

            /**
             * @brief Define o endereço MAC.
             * 
             * @param addr O endereço MAC como array de bytes.
             */
            void Set(const byte* addr);

            /**
             * @brief Converte um endereço MAC representado como string para array de bytes.
             * 
             * @param mac O endereço MAC como string.
             */
            void StrToArray(std::string mac);

            byte address[6] = {}; // Armazena um endereço MAC como array de bytes
        };
    }
}

#endif //MAC_HPP