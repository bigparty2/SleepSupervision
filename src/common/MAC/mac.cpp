#include "mac.hpp"

using namespace ss;

network::MAC::MAC(std::string addr)
{
    this->StrToArray(addr);
}

network::MAC::MAC(byte* addr)
{
    this->address[0] = addr[0];
    this->address[1] = addr[1];
    this->address[2] = addr[2];
    this->address[3] = addr[3];
    this->address[4] = addr[4];
    this->address[5] = addr[5];
}

network::MAC& network::MAC::operator=(const std::string& macAddr)
{
    this->StrToArray(macAddr);

    return *this;
}

network::MAC& network::MAC::operator=(const MAC& newMac)
{
    this->address[0] = newMac.address[0];
    this->address[1] = newMac.address[1];
    this->address[2] = newMac.address[2];
    this->address[3] = newMac.address[3];
    this->address[4] = newMac.address[4];
    this->address[5] = newMac.address[5];

    return *this;
}

bool network::MAC::operator==(const MAC& toCompare)
{
    return  toCompare.address[0] == this->address[0] and
            toCompare.address[1] == this->address[1] and
            toCompare.address[2] == this->address[2] and
            toCompare.address[3] == this->address[3] and
            toCompare.address[4] == this->address[4] and
            toCompare.address[5] == this->address[5];
}       

byte* network::MAC::Get()
{
    return this->address;
}

void network::MAC::StrToArray(std::string mac)
{
    auto macLen = mac.size();

    for (size_t i = 0, j = 0; i < macLen; j++) 
    {
        this->address[j] = static_cast<char>(std::stoi(mac.substr(i, 2), 0, 16));

        i += 2;

        if (i != macLen and mac.at(i) == ':') 
            ++i;
    }
}

std::string network::MAC::ToString()
{
    char buffer[18];

    sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", this->address[0], this->address[1], this->address[2], this->address[3], this->address[4], this->address[5]);

    return std::string(buffer);
}

std::string network::MAC::ToString(byte* addr)
{
    char buffer[18];

    sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    return std::string(buffer);
}

void network::MAC::GetMACAddrr(std::string interface)
{
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    MAC mac;

    if(getifaddrs(&ifAddrStruct) == -1)
    {
        logger::GetInstance().Error(__PRETTY_FUNCTION__, "Falha ao executar getifaddrs. Erro: " + std::string(std::strerror(errno)));
        throw std::runtime_error("Falha ao executar getifaddrs. Erro: " + std::string(std::strerror(errno)));    
    }

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) 
    {
        if (!ifa->ifa_addr)
            continue;

        // mac address
        else if(ifa->ifa_addr->sa_family == AF_PACKET)
        {
            if(std::string(ifa->ifa_name).find("eth") != std::string::npos or std::string(ifa->ifa_name).find("enp") != std::string::npos)
            {
                struct sockaddr_ll *s = (struct sockaddr_ll*)ifa->ifa_addr;

                *this = MAC(s->sll_addr);

                break;
            }
        }
    }

    if (ifAddrStruct!=NULL) 
        freeifaddrs(ifAddrStruct);
}
