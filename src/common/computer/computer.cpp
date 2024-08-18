#include "computer.hpp"

using namespace ss;

computer::computer(std::string name, network::MAC macAddr, network::IPV4 ipv4, computerStatus status)
{
    this->name = name;
    this->macAddr = macAddr;
    this->ipv4 = ipv4;
    this->status = status;
}

computer::computer(computerData data)
{
    this->name = data.name;
    this->macAddr = data.macAddr;
    this->ipv4 = data.ipv4;
    this->status = static_cast<ss::computer::computerStatus>(data.status);
    this->id = data.id;
    this->isLeader = data.isLeader;
}

std::string computer::GetComputerName()
{
    char hostname[256];
    
    gethostname(hostname, 256);

    return std::string(hostname);
}

std::string computer::StatusToStringEN(computer::computerStatus status)
{
    std::string listNames[] = {"Awake", "Sleep", "Unknown"};

    return std::string(listNames[status]);
}

std::string computer::StatusToStringEN()
{
    return this->StatusToStringEN(this->status);
}

std::string computer::StatusToStringBR(computer::computerStatus status)
{
    std::string listNames[] = {"Acordado", "Dormindo", "Desconhecido"};

    return std::string(listNames[status]);
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

    logger::GetInstance().Debug(__PRETTY_FUNCTION__, this->name + "|" + this->ipv4.ToString() + "|" + this->macAddr.ToString());
}

std::string computer::GetName() const
{
    return this->name;
}

network::MAC computer::GetMAC() const
{
    return this->macAddr;
}

network::IPV4 computer::GetIPV4() const
{
    return this->ipv4;
}

computer::computerStatus computer::GetStatus() const
{
    return this->status;
}

void computer::SetStatus(computer::computerStatus status)
{
    this->status = status;
}

computer::computerData computer::ToComputerData()
{
    computerData data;

    std::strncpy(data.name, this->name.substr(0, sizeof(data.name)).c_str(), sizeof(data.name));
    for(int i = 0; i < 6; i++)
        data.macAddr[i] = this->macAddr.Get()[i];
    data.ipv4 = this->ipv4.Get();
    data.status = static_cast<uint8_t>(this->status);
    data.id = this->id;
    data.isLeader = this->isLeader;

    return data;
}

int computer::GetID() const
{
    return this->id;
}

void computer::SetID(int id)
{
    this->id = id;
}
