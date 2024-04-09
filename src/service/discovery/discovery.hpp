#ifndef DISCOVERY_HPP
#define DISCOVERY_HPP

#include "../manager/manager.hpp"
#include "../../common/computer/computer.hpp"
#include "../../common/socket/socket.hpp"
#include "../../common/packet/packet.hpp"

namespace ss
{
    namespace discovery
    {
        
        class DiscoverySubservice
        {
            public:
            
            DiscoverySubservice(manager::computersManager &computersManager);
            ~DiscoverySubservice();
            
            void Start(bool isServer = false);
            void Stop();

            private:

            void clientRun();
            void serverRun();

            //Instancia da classe de gerenciamento de computadores
            manager::computersManager* computersManager;

            //Porta padrao para descoberta
            static const uint16_t DISCOVERY_PORT_SERVER = 45001;
            static const uint16_t DISCOVERY_PORT_CLIENT_INIT = 45002;
            static const uint16_t DISCOVERY_PORT_CLIENT_END = 45102;

            //Tempo de espera para resposta padrao
            timeval TIMEOUT;


        };
    }
}

#endif //DISCOVERY_HPP