#ifndef MONITOR_HPP
#define MONITOR_HPP

#include <vector>
#include "../manager/manager.hpp"
#include "../../common/packet/packet.hpp"
#include "../../common/socket/socket.hpp"
#include "../../common/thread/thread.hpp"
#include "../../common/logger/logger.hpp"

#include "../../common/comunication/comunication.hpp"
#include "../../common/comunication/comunicationPacket.hpp"
#include "../../common/comunication/comunicationType.hpp"

namespace ss
{
    namespace monitor
    {
        /**
         * @brief Classe que representa o subserviço de monitoramento.
         */
        class MonitorSubservice
        {
            public:
            /**
             * @brief Constrói um objeto MonitorSubservice.
             * 
             * @param computersManager Referência para o objeto computersManager para gerenciar os computadores.
             */
            MonitorSubservice(manager::computersManager& computersManager);

            /**
             * @brief Destrói o objeto MonitorSubservice.
             */
            ~MonitorSubservice();

            /**
             * @brief Inicia o subserviço de monitoramento.
             * 
             * @param isServer Flag que indica se deve iniciar como servidor ou não. O padrão é falso.
             */
            void Start(bool isServer = false);

            /**
             * @brief Para o subserviço de monitoramento.
             */
            void Stop();

            private:
            /**
             * @brief Executa o processo do cliente.
             */
            void clientRun();
            void clientRun_V2();

            /**
             * @brief Executa o processo do servidor.
             */
            void serverRun();
            void serverRun_V2();

            /**
             * @brief Atualiza a lista de computadores para monitorar.
             */
            void UpdateComputersToMonior();

            // Instância da classe computersManager para gerenciar os computadores
            manager::computersManager* computersManager;

            // Última atualização da lista de computadores
            uint64_t lastUpdate;

            // Tempo limite para receber pacotes
            timeval TIMEOUT;

            struct monitorFail
            {
                computer Computer;
                uint8_t failCount;
            };

            // Lista de monitores com falha
            std::vector<monitorFail> failedMonitors;

            // Controle de falhas do host
            uint8_t hostFailCount;

            // Máximo de falhas permitidas
            static const uint8_t MAX_FAILS = 3;

            static const uint16_t MONITOR_PORT_SERVER = 45103;
            static const uint16_t MONITOR_PORT_CLIENT = 45104;
        };
    }
}

#endif //MONITOR_HPP