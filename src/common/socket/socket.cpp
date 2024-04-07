#include "socket.hpp"

using namespace ss;

network::Socket::Socket(int protocol) :
    protocol(protocol),
    descriptor(socket(AF_INET, SOCK_DGRAM, protocol))
{
    if(this->descriptor == SOCKET_ERROR)
    {
        //TODO: Inclir erro na classe LOG
        throw std::runtime_error("Falha ao abrir o socket. Erro: " + std::string(std::strerror(errno)));
    }
}

network::Socket::~Socket()
{
    close(this->descriptor);
}

void network::Socket::SetConfig(int optname, int optaval)
{
    if(setsockopt(this->descriptor, SOL_SOCKET, optname, &optaval, sizeof(optaval)) == SOCKET_ERROR)
    {
        //TODO: Incluir erro na classe LOG
        throw std::runtime_error("Falha ao abrir o socket. Erro: " + std::string(std::strerror(errno)));       
    }
}

void network::Socket::Send(const char* data, size_t dataSize, uint16_t port, uint32_t address)
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(address);

    auto bytesSend = sendto(this->descriptor, data, dataSize, 0, reinterpret_cast<sockaddr*>(&addr), sizeof addr);

    if(bytesSend == SOCKET_ERROR)
    {
        //TODO: Incluir erro na classe LOG
        throw std::runtime_error("Falha ao enviar o pacote. Erro: " + std::string(std::strerror(errno)));    
    }
    else if(bytesSend != dataSize)
    {
        //TODO: Incluir erro na classe LOG
        throw std::runtime_error("Enviado um numero diferentes de bytes do esperado. Erro: " + std::string(std::strerror(errno)));  
    }
}

void network::Socket::Send(const char* data, size_t dataSize, uint16_t port, char* address)
{
    this->Send(data, dataSize, port, inet_addr(address));
}

void network::Socket::Send(packet& packet, uint16_t port, uint32_t address)
{
    this->Send((char*)packet.GetPacketData(), packet.GetPacketSize(), port, address);
}

void network::Socket::Send(packet& packet, uint16_t port, char* address)
{
    this->Send((char*)packet.GetPacketData(), packet.GetPacketSize(), port, address);
}


void network::Socket::Bind(uint16_t port)
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    std::memset(addr.sin_zero, 0, 8);

    if (bind(this->descriptor, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0)
    {
        //TODO: Incluir erro na classe LOG
        throw std::runtime_error("Falha ao criar bind. Erro: " + std::string(std::strerror(errno)));
    }

    this->bindIsSet = true;
}

ss::network::packet network::Socket::receiveSocket()
{
    if(!this->bindIsSet)
    {
        //TODO: Incluir erro na classe LOG
        throw std::runtime_error("Bind não foi definido.");
    }

    byte buffer[packet::GetPacketSize()];
    std::memset(buffer, 0, packet::GetPacketSize());

    struct sockaddr_in client;
    socklen_t clientLen = sizeof(struct sockaddr_in);

    auto bytesReceived = recvfrom(this->descriptor, buffer, packet::GetPacketSize(), 0, reinterpret_cast<sockaddr*>(&client), &clientLen);

    if(bytesReceived == SOCKET_ERROR)
    {   
        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {
            //Timeout
            return ss::network::packet(); 
        }
        else
        {
            //TODO: Incluir erro na classe LOG
            throw std::runtime_error("Falha ao receber pacote. Erro: " + std::string(std::strerror(errno)));
        }
    }

    ss::network::packet::_packet* packet = new ss::network::packet::_packet;
    std::memcpy(packet, buffer, packet::GetPacketSize());

    ss::network::packet ret(*packet);

    delete packet;

    return ret;
}
