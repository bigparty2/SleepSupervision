#ifndef MONITOR_HPP
#define MONITOR_HPP

#include <vector>
#include "../manager/manager.hpp"
#include "../../common/packet/packet.hpp"
#include "../../common/socket/socket.hpp"
#include "../../common/thread/thread.hpp"

namespace ss
{
    namespace monitor
    {
        class MonitorSubservice
        {
            public:
            /**
             * @brief Constructs a MonitorSubservice object.
             */
            MonitorSubservice(manager::computersManager& computersManager);

            /**
             * @brief Destroys the MonitorSubservice object.
             */
            ~MonitorSubservice();

            /**
             * @brief Starts the monitor subservice.
             * 
             * @param isServer Flag indicating whether to start as a server or not. Default is false.
             */
            void Start(bool isServer = false);

            /**
             * @brief Stops the monitor subservice.
             */
            void Stop();

            private:
            /**
             * @brief Runs the client process.
             */
            void clientRun();

            /**
             * @brief Runs the server process.
             */
            void serverRun();

            void UpdateComputersToMonior();

            // Instance of the computersManager class for managing computers
            manager::computersManager* computersManager;

            // Last update of the computer list
            uint64_t lastUpdate;

            // Timeout for receiving packets
            timeval TIMEOUT;

            struct monitorFail
            {
                computer Computer;
                uint8_t failCount;
            };

            // List of failed monitors
            std::vector<monitorFail> failedMonitors;

            // Maximo de falhas
            static const uint8_t MAX_FAILS = 3;

            static const uint16_t MONITOR_PORT_SERVER = 45103;
            static const uint16_t MONITOR_PORT_CLIENT = 45104;
        };
    }
}

#endif //MONITOR_HPP