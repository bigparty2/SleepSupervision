#ifndef COMPUTER_HPP
#define COMPUTER_HPP

#include <string>
#include <unistd.h>

#include "../logger/logger.hpp"
#include "../MAC/mac.hpp"
#include "../IPV4/ipv4.hpp"

namespace ss
{
    //Classe para armazenamento dos dados de um computador
    class computer
    {
        public:

        //Enumeração dos posiveis estados do computador
        enum computerStatus 
        {
            awake = 0, 
            sleep = 1, 
            unknown = 2
        };

        // Conversor do status para string
        std::string StatusToStringEN();
        std::string StatusToStringBR();
        static std::string StatusToStringEN(computerStatus status);
        static std::string StatusToStringBR(computerStatus status);

        // Pegar dados do computador local
        void GetComputerInfo();

        //Constructor
        computer(){};
        computer(std::string name, network::MAC macAddr, network::IPV4 ipv4, computerStatus status);

        // Pegar o nome do computador local
        static std::string GetComputerName(); 

        //TODO: Implementar metodos get/set para atributos ou localizar alternativas modernas para tal
        // private:

        mutable std::string name;
        mutable network::MAC macAddr;
        mutable network::IPV4 ipv4;
        computerStatus status;
    };

    typedef std::vector<ss::computer> computers;
}

#endif //COMPUTER_HPP