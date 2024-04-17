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
        /**
         * @brief Classe que representa um endereço IPv4.
         */
        class IPV4
        {
        public:

            /**
             * @brief Construtor padrão da classe IPV4.
             */
            IPV4(){}

            /**
             * @brief Construtor da classe IPV4 que recebe um endereço IPv4 em formato inteiro.
             * @param addr O endereço IPv4 em formato inteiro.
             */
            IPV4(uint32_t addr);

            /**
             * @brief Construtor da classe IPV4 que recebe um endereço IPv4 em formato de string.
             * @param addr O endereço IPv4 em formato de string.
             */
            IPV4(const std::string& addr);

            /**
             * @brief Sobrecarga do operador de atribuição para um endereço IPv4 em formato de string.
             * @param addr O endereço IPv4 em formato de string.
             * @return Uma referência para o objeto IPV4 atualizado.
             */
            IPV4& operator=(const std::string&);

            /**
             * @brief Sobrecarga do operador de atribuição para um endereço IPv4 em formato inteiro.
             * @param addr O endereço IPv4 em formato inteiro.
             * @return Uma referência para o objeto IPV4 atualizado.
             */
            IPV4& operator=(const uint32_t&);

            /**
             * @brief Sobrecarga do operador de igualdade para comparar dois endereços IPv4.
             * @param other O outro endereço IPv4 a ser comparado.
             * @return True se os endereços IPv4 forem iguais, False caso contrário.
             */
            bool operator==(const IPV4&);

            /**
             * @brief Retorna o endereço IPv4 como uma string.
             * @return O endereço IPv4 como uma string.
             */
            std::string ToString();

            /**
             * @brief Retorna o endereço IPv4 como uma wide string.
             * @return O endereço IPv4 como uma wide string.
             */
            std::wstring ToWString();

            /**
             * @brief Retorna o endereço IPv4 como um inteiro.
             * @return O endereço IPv4 como um inteiro.
             */
            uint32_t Get();

            /**
             * @brief Obtém o endereço IPv4 local para uma interface de rede específica.
             * @param interface A interface de rede para a qual se deseja obter o endereço IPv4 local.
             */
            void GetIPV4(std::string interface);

        private:

            /**
             * @brief Converte um endereço IPv4 em formato inteiro para um array de bytes.
             * @param addr O endereço IPv4 em formato inteiro.
             */
            void IntToByteArr(uint32_t addr);

            /**
             * @brief Converte um array de bytes para um endereço IPv4 em formato inteiro.
             * @return O endereço IPv4 em formato inteiro.
             */
            uint32_t ByteArrToInt();

            /**
             * @brief Valida um endereço IPv4 em formato de string.
             * @param addr O endereço IPv4 em formato de string.
             */
            void IPV4Validation(const std::string& addr);

            /**
             * @brief Armazena o endereço IPv4 em formato de inteiro (network byte order).
             */
            uint32_t _address = 0;
        };
    }
}

#endif //IPV4_HPP