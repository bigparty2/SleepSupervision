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

#include "../../../common/types.hpp"

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
            void SetConfig(int optname, int optaval);

            //Transmite dado configurado
            void Send(const char* data, size_t dataSize, uint16_t port, uint32_t address);
            void Send(const char* data, size_t dataSize, uint16_t port, char* address);

            // Recebe dados
            void receive();
            // void receive()

            //operadores
            Socket& operator=(const Socket&) = delete;

            private:

            static const int SOCKET_ERROR = -1;
            const int protocol;
            const int descriptor;
        };
    }
}

#endif //SOCKET_HPP