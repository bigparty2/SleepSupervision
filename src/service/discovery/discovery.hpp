#ifndef DISCOVERY_HPP
#define DISCOVERY_HPP

#include "../manager/manager.hpp"
#include "../../common/computer/computer.hpp"
#include "../../common/socket/socket.hpp"
#include "../../common/packet/packet.hpp"
#include "../../common/thread/thread.hpp"

namespace ss
{
    namespace discovery
    {
        
        /**
         * @brief Classe que representa o subserviço de descoberta.
         * 
         * Esta classe é responsável por gerenciar a descoberta de computadores na rede.
         * Ela permite iniciar e parar o subserviço, bem como executar o subserviço como cliente ou servidor.
         */
        class DiscoverySubservice
        {
            public:
            
            /**
             * @brief Construtor da classe DiscoverySubservice.
             * 
             * @param computersManager Referência para a classe de gerenciamento de computadores.
             */
            DiscoverySubservice(manager::computersManager &computersManager);
            
            /**
             * @brief Destrutor da classe DiscoverySubservice.
             */
            ~DiscoverySubservice();
            
            /**
             * @brief Inicia o subserviço de descoberta.
             * 
             * @param isServer Indica se o subserviço deve ser executado como servidor (true) ou cliente (false).
             */
            void Start(bool isServer = false);
            
            /**
             * @brief Para o subserviço de descoberta.
             */
            void Stop();

            private:

            /**
             * @brief Executa o subserviço de descoberta como cliente.
             */
            void clientRun();
            
            /**
             * @brief Executa o subserviço de descoberta como servidor.
             */
            void serverRun();

            /**
             * @brief Instância da classe de gerenciamento de computadores.
             */
            manager::computersManager* computersManager;

            /**
             * @brief Porta padrão para descoberta.
             */
            static const uint16_t DISCOVERY_PORT_SERVER = 45001;
            static const uint16_t DISCOVERY_PORT_CLIENT_INIT = 45002;
            static const uint16_t DISCOVERY_PORT_CLIENT_END = 45102;

            /**
             * @brief Tempo de espera para resposta padrão.
             */
            timeval TIMEOUT;


        };
    }
}

#endif //DISCOVERY_HPP