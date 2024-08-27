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

template<typename T>
void network::Socket::SetConfig(int optname, T optaval)
{
    if(setsockopt(this->descriptor, SOL_SOCKET, optname, &optaval, sizeof(optaval)) == SOCKET_ERROR)
    {
        //TODO: Incluir erro na classe LOG
        throw std::runtime_error("Falha ao definir uma configuração do socket. Config: " + std::to_string(optname) + ". Erro: " + std::string(std::strerror(errno)));       
    }
}

// Instancia explicita de SetConfig
template void ss::network::Socket::SetConfig<int>(int optname, int optaval);
template void ss::network::Socket::SetConfig<timeval>(int optname, timeval optaval);

void network::Socket::Send(const char* data, size_t dataSize, uint16_t port, uint32_t address)
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(address);

    auto bytesSend = sendto(this->descriptor, data, dataSize, 0, reinterpret_cast<sockaddr*>(&addr), sizeof addr);

    if(bytesSend == SOCKET_ERROR)
    {
        if(errno == ENETUNREACH)
        {
            logger::GetInstance().Error(__PRETTY_FUNCTION__ ,"Falha ao enviar o pacote. Erro: " + std::string(std::strerror(errno)));
            return;
        }

        //TODO: Implementar verificao de timeout

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

void network::Socket::Send(ss::network::ComunicationPacket packet)
{
    this->Send((char*)packet.GetPacketData(), packet.GetPacketMSGSize(), packet.GetPacketMSG().portDest, packet.GetPacketMSG().destPC.ipv4);
}

void network::Socket::Bind(uint16_t &port, uint16_t range)
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    std::memset(addr.sin_zero, 0, 8);

    while(bind(this->descriptor, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0)
    {
        if (errno == EADDRINUSE)
        {
            if(port < range)
            {
                // Porta já em uso, tente a próxima porta
                port++;
                addr.sin_port = htons(port);
            }
            else
            {
                //TODO: Incluir erro na classe LOG
                throw std::runtime_error("Nenhuma porta disponível. Erro: " + std::string(std::strerror(errno)));
            }
        }
        else
        {
        //TODO: Incluir erro na classe LOG
        throw std::runtime_error("Falha ao criar bind. Erro: " + std::string(std::strerror(errno)));
        }   
    }

    this->bindIsSet = true;
}

ss::network::ComunicationPacket network::Socket::Receive()
{   
    if(!this->bindIsSet)
    {
        //TODO: Incluir erro na classe LOG
        throw std::runtime_error("Bind não foi definido.");
    }

    byte buffer[ComunicationPacket::GetPacketMSGSize()];
    std::memset(buffer, 0, ComunicationPacket::GetPacketMSGSize());

    struct sockaddr_in client;
    socklen_t clientLen = sizeof(struct sockaddr_in);

    auto bytesReceived = recvfrom(this->descriptor, buffer, ComunicationPacket::GetPacketMSGSize(), 0, reinterpret_cast<sockaddr*>(&client), &clientLen);

    if(bytesReceived == SOCKET_ERROR)
    {   
        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {
            //Timeout
            // logger::GetInstance().Log(__PRETTY_FUNCTION__ ,"Timeout");
            throw std::runtime_error("Timeout"); 
        }
        else if (errno == EINTR)
        {
            logger::GetInstance().Log(__PRETTY_FUNCTION__ ,"Socket Interrupted system call");
            throw std::runtime_error("Socket Interrupted system call");
        }
        else if (errno == EMSGSIZE)
        {
            logger::GetInstance().Log(__PRETTY_FUNCTION__ ,"Socket received message too long");
            throw std::runtime_error("Socket received message too long");
        }
        else if (errno == EFAULT)
        {
            logger::GetInstance().Log(__PRETTY_FUNCTION__ ,"Erro no buffer. Erro: " + std::string(std::strerror(errno)));
            throw std::runtime_error("Erro no buffer. Erro: " + std::string(std::strerror(errno)));
        }
        else
        {
            logger::GetInstance().Log(__PRETTY_FUNCTION__ ,"Falha ao receber pacote. Erro: " + std::string(std::strerror(errno)));
            throw std::runtime_error("Falha ao receber pacote. Erro: " + std::string(std::strerror(errno)));
        }
    } else if (bytesReceived == ComunicationPacket::GetPacketMSGSize())
    {
        ComunicationPacket::__packetMsg *packetMsg = new ComunicationPacket::__packetMsg;
        std::memcpy(packetMsg, buffer, ComunicationPacket::GetPacketMSGSize());
        ComunicationPacket::__packetMsg packet = *packetMsg;
        delete packetMsg;
        return packet;


    } else if (bytesReceived == ComunicationPacket::GetPacketMSGSize())
    {
        ComunicationPacket::__packetMsg *packetMsg = new ComunicationPacket::__packetMsg;
        std::memcpy(packetMsg, buffer, ComunicationPacket::GetPacketMSGSize());
        ComunicationPacket::__packetMsg packet = *packetMsg;
        delete packetMsg;
        return packet;
    
    } else
    {
        throw std::runtime_error("Tamanho de pacote inválido");
    }
}

ss::network::packet network::Socket::receivePacket()
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
            // logger::GetInstance().Log(__PRETTY_FUNCTION__ ,"Timeout");
            return ss::network::packet(); 
        }
        else if (errno == EINTR)
        {
            logger::GetInstance().Log(__PRETTY_FUNCTION__ ,"Socket Interrupted system call");
            return ss::network::packet();
        }
        else if (errno == EMSGSIZE)
        {
            logger::GetInstance().Log(__PRETTY_FUNCTION__ ,"Socket received message too long");
            return ss::network::packet();
        }
        else if (errno == EFAULT)
        {
            logger::GetInstance().Log(__PRETTY_FUNCTION__ ,"Erro no buffer. Erro: " + std::string(std::strerror(errno)));
            throw std::runtime_error("Erro no buffer. Erro: " + std::string(std::strerror(errno)));
        }
        else
        {
            logger::GetInstance().Log(__PRETTY_FUNCTION__ ,"Falha ao receber pacote. Erro: " + std::string(std::strerror(errno)));
            throw std::runtime_error("Falha ao receber pacote. Erro: " + std::string(std::strerror(errno)));
        }
    }

    ss::network::packet::_packet* packet = new ss::network::packet::_packet;
    std::memcpy(packet, buffer, packet::GetPacketSize());

    ss::network::packet ret(*packet);

    delete packet;

    return ret;
}
