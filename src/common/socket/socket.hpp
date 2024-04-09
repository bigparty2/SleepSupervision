#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <string>
#include <stdexcept>


// #include <stdexcept>
#include <cstring>
#include <ostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <netpacket/packet.h>
#include <errno.h>
#include <unistd.h>
#include <stdexcept>
#include <algorithm>

#include "../types.hpp"
#include "../packet/packet.hpp"
#include "../logger/logger.hpp"

namespace ss
{
    namespace network
    {

        //Abstração do use geral de sockets
        class Socket
        {
            public:

            //constructor
            Socket(const int protocol);

            //destructor
            ~Socket();

            //Define as configurações do socket
            // template<typename T>
            // void SetConfig(int optname, T optaval);
            void SetConfig(int optname, int optaval);
            void SetConfig(int optname, timeval optaval);


            //Associação a uma porta
            void Bind(uint16_t &port, uint16_t range = 0);

            //Transmite dado configurado
            void Send(const char* data, size_t dataSize, uint16_t port, uint32_t address);
            void Send(const char* data, size_t dataSize, uint16_t port, char* address);
            void Send(packet& packet, uint16_t port, uint32_t address);
            void Send(packet& packet, uint16_t port, char* address);

            // Recebe dados
            packet receiveSocket();

            //operadores
            Socket& operator=(const Socket&) = delete;

            private:

            bool bindIsSet = false;
            static const int SOCKET_ERROR = -1;
            const int protocol;
            const int descriptor;
        };
    }
}

#endif //SOCKET_HPP