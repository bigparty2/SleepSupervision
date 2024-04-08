#include "wakeOnLan.hpp"

using namespace ss;

network::wakeOnLan::magicPacket::magicPacket(const std::string &macAddr)
{
    this->mp = this->Build(macAddr);
}

std::string network::wakeOnLan::magicPacket::Build(const std::string &macAddr)
{
    std::string addrHex;
    size_t macAddrLen = macAddr.length();

    for (size_t i = 0; i < macAddrLen;) 
    {
        addrHex += static_cast<char>(stoi(macAddr.substr(i, 2), 0, 16));

        i += 2;

        if (i != macAddrLen and macAddr.at(i) == ':') 
            ++i;
    }

    if (addrHex.length() != 6)
    {
        //TODO: Utilizar classe log para registro do erro        

        throw std::runtime_error("O endereço mac " + macAddr + " não é valido");    
    }

    return std::string(6, 0xFF) + string::Repeat(16, addrHex);
}

const char* network::wakeOnLan::magicPacket::Get()
{
    return this->mp.c_str();
}

byte network::wakeOnLan::magicPacket::Length()
{
    return magicPacket::MAGIC_PACKET_SIZE;
}


std::string network::wakeOnLan::magicPacket::ToString(const std::string &mp)
{
    std::ostringstream oss;

    for(const auto &hex : mp)
        oss << std::hex << std::uppercase << ((int)hex & 0xFF);

    return oss.str();
}

/**
 * Converts the magic packet to a string representation.
 * 
 * @return The string representation of the magic packet.
 */
std::string network::wakeOnLan::magicPacket::ToString()
{
    return this->ToString(this->mp);
}

/**
 * @brief Sends a Wake-on-LAN magic packet to wake up a device with the specified MAC address.
 * 
 * @param macAddress The MAC address of the device to wake up.
 * @param port The port number to send the magic packet to.
 */
void network::wakeOnLan::Awake(const std::string &macAddress, const byte &port)
{
    //Inicialização e construção do pacote mágico
    magicPacket mp(macAddress);

    //Criando socket
    Socket sock(IPPROTO_UDP);

    //Opções do socket brodcast
    sock.SetConfig(SO_BROADCAST, 1);

    //Enviando o pacote magico
    sock.Send(mp.Get(), mp.Length(), port, INADDR_BROADCAST);

    return;
}
