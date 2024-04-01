#include "computer.hpp"

using namespace ss;

computer::computer(std::string name, network::MAC macAddr, network::IPV4 ipv4, computerStatus status)
{
    this->name = name;
    this->macAddr = macAddr;
    this->ipv4 = ipv4;
    this->status = status;
}

std::string computer::GetComputerName()
{
    char hostname[256];
    
    gethostname(hostname, 256);

    return std::string(hostname);
}

std::string computer::StatusToStringEN(computer::computerStatus status)
{
    std::string toReturn;

    switch (status)
    {
    case 0:
        toReturn = "Awake";
        break;

    case 1:
        toReturn = "Sleep";
        break;

    case 2:
        toReturn = "Unknown";
        break;
    
    default:
        throw "not implemented";
    }

    return toReturn;
}

std::string computer::StatusToStringEN()
{
    return this->StatusToStringEN(this->status);
}

std::string computer::StatusToStringBR(computer::computerStatus status)
{
    std::string toReturn;

    switch (status)
    {
    case 0:
        toReturn = "Acordado";
        break;

    case 1:
        toReturn = "Dormindo";
        break;

    case 2:
        toReturn = "Desconhecido";
        break;
    
    default:
        throw "not implemented";
    }

    return toReturn;
}

std::string computer::StatusToStringBR()
{
    return this->StatusToStringBR(this->status);
}

void computer::GetComputerInfo()
{
    this->name = computer::GetComputerName();
    this->ipv4.GetIPV4("eth_|enp_s_");
    this->macAddr.GetMACAddrr("eth_|enp_s_");
    this->status = computer::computerStatus::awake;

    logger::GetInstance().Log(__PRETTY_FUNCTION__, this->name + "|" + this->ipv4.ToString() + "|" + this->macAddr.ToString());
}

