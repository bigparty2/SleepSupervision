#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "../types.hpp"
#include "../packet/packet.hpp"
#include "../logger/logger.hpp"

namespace ss
{
    namespace network
    {
        /**
         * @brief The Socket class represents a socket for network communication.
         * 
         * This class provides methods for configuring, binding, sending, and receiving data over a network.
         */
        class Socket
        {
            public:
            /**
             * @brief Constructs a Socket object with the specified protocol.
             * @param protocol The protocol to be used by the socket.
             */
            Socket(int protocol);

            ~Socket();

            /**
             * @brief Sets the configuration option for the socket.
             * 
             * @tparam T The type of the option value.
             * @param optname The name of the option to set.
             * @param optaval The value to set for the option.
             */
            template<typename T>
            void SetConfig(int optname, T optaval);

            /**
             * @brief Binds the socket to a specific port.
             *
             * @param port The port number to bind the socket to. If range is defined, it will contain the port that was bound.
             * @param range The range of ports to use for try binding (optional, default is 0).
             */
            void Bind(uint16_t &port, uint16_t range = 0);

            /**
             * @brief Sends data over a socket to a specified address and port.
             *
             * @param data The pointer to the data to be sent.
             * @param dataSize The size of the data to be sent.
             * @param port The port number to send the data to.
             * @param address The IP address to send the data to.
             */
            void Send(const char* data, size_t dataSize, uint16_t port, uint32_t address);

            /**
             * @brief Sends data over a socket to a specified address and port.
             *
             * @param data The data to be sent.
             * @param dataSize The size of the data in bytes.
             * @param port The port number to send the data to.
             * @param address The address to send the data to (IPV4).
             */
            void Send(const char* data, size_t dataSize, uint16_t port, char* address);
    
            /**
             * @brief Sends a packet over a socket to the specified address and port.
             *
             * @param packet The packet to send.
             * @param port The port to send the packet to.
             * @param address The address to send the packet to.
             */
            void Send(packet& packet, uint16_t port, uint32_t address);
                    
            /**
             * @brief Sends a packet over a socket to the specified address and port.
             *
             * @param packet The packet to send.
             * @param port The port to send the packet to.
             * @param address The address to send the packet to.
             */
            void Send(packet& packet, uint16_t port, char* address);

            /**
             * @brief Receives a packet from the socket.
             *
             * @return The received packet.
             */
            packet receivePacket();

            //operator = denided
            Socket& operator=(const Socket&) = delete;

            private:

            /**
             * @brief The descriptor of the socket.
             */
            const int descriptor;
            
            /**
             * @brief Indicates whether the socket is bound to a port.
             */
            bool bindIsSet = false;
            
            /**
             * @brief The error code for a socket error.
             */
            static const int SOCKET_ERROR = -1;
            
            /**
             * @brief The protocol to be used by the socket.
             */
            const int protocol;
        };
    }
}

#endif //SOCKET_HPP