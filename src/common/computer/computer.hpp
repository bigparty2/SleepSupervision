#ifndef COMPUTER_HPP
#define COMPUTER_HPP

#include <string>
#include <vector>
#include <unistd.h>

#include "../logger/logger.hpp"
#include "../MAC/mac.hpp"
#include "../IPV4/ipv4.hpp"

namespace ss
{
    /**
     * @brief Classe que representa um computador.
     */
    class computer
    {
        public:

        /**
         * @brief struct com tipos básicos para armazenamento de um dado de um computador.
         */ 
        struct computerData
        {
            char name[64]; /**< O nome do computador. */
            byte macAddr[6]; /**< O endereço MAC do computador. */
            uint32_t ipv4; /**< O endereço IPv4 do computador. */
            uint8_t status; /**< O estado do computador. */
        };

        /**
         * @brief Enumeração que define os possíveis estados de um computador.
         */
        enum computerStatus 
        {
            awake = 0, /**< O computador está acordado. */
            sleep = 1, /**< O computador está em modo de suspensão. */
            unknown = 2 /**< O estado do computador é desconhecido. */
        };

        /**
         * @brief Converte o estado do computador para uma string em inglês.
         * @return A string que representa o estado do computador em inglês.
         */
        std::string StatusToStringEN();

        /**
         * @brief Converte o estado do computador para uma string em português.
         * @return A string que representa o estado do computador em português.
         */
        std::string StatusToStringBR();

        /**
         * @brief Converte o estado do computador para uma string em inglês.
         * @param status O estado do computador.
         * @return A string que representa o estado do computador em inglês.
         */
        static std::string StatusToStringEN(computerStatus status);

        /**
         * @brief Converte o estado do computador para uma string em português.
         * @param status O estado do computador.
         * @return A string que representa o estado do computador em português.
         */
        static std::string StatusToStringBR(computerStatus status);

        /**
         * @brief Obtém as informações do computador.
         */
        void GetComputerInfo();

        /**
         * @brief Construtor padrão da classe computer.
         */
        computer(){};

        /**
         * @brief Construtor da classe computer.
         * @param name O nome do computador.
         * @param macAddr O endereço MAC do computador.
         * @param ipv4 O endereço IPv4 do computador.
         * @param status O estado do computador.
         */
        computer(std::string name, network::MAC macAddr, network::IPV4 ipv4, computerStatus status);

        /**
         * @brief Construtor da classe computer.
         * @param data A estrutura ComputerData com os dados do computador.
         */
        computer(computerData data);

        /**
         * @brief Obtém o nome do computador.
         * @return O nome do computador.
         */
        static std::string GetComputerName(); 

        /**
         * @brief Obtém o nome do computador.
         * @return O nome do computador.
         */
        std::string GetName() const;

        /**
         * @brief Obtém o endereço MAC do computador.
         * @return O endereço MAC do computador.
         */
        network::MAC GetMAC() const;

        /**
         * @brief Obtém o endereço IPv4 do computador.
         * @return O endereço IPv4 do computador.
         */
        network::IPV4 GetIPV4() const;

        /**
         * @brief Obtém o estado do computador.
         * @return O estado do computador.
         */
        computerStatus GetStatus() const;

        /**
         * @brief Define o estado do computador.
         * @param status O estado do computador.
         */
        void SetStatus(computerStatus status);

        /**
         * @brief Converte os dados da classe Computer para a estrutura ComputerData de tipos básicos
         * @return A estrutura ComputerData com os dados do computador.
         */
        computerData ToComputerData();

        private: 

        std::string name; /**< O nome do computador. */
        network::MAC macAddr; /**< O endereço MAC do computador. */
        network::IPV4 ipv4; /**< O endereço IPv4 do computador. */
        computerStatus status; /**< O estado do computador. */
    };

    typedef std::vector<ss::computer> computers; /**< Um vetor de computadores. */
}

#endif //COMPUTER_HPP