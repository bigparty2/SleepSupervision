#include "ipv4.hpp"

using namespace ss;

network::IPV4::IPV4(uint32_t addr)
{
    this->_address = addr;
}

network::IPV4::IPV4(const std::string& addr)
{
    IPV4Validation(addr);

    this->_address = htonl(inet_addr(addr.c_str()));
}

network::IPV4& network::IPV4::operator=(const std::string& addr)
{
    IPV4Validation(addr);

    this->_address = htonl(inet_addr(addr.c_str()));

    return *this;
}

network::IPV4& network::IPV4::operator=(const uint32_t& addr)
{
    this->_address = addr;

    return *this;
}

bool network::IPV4::operator==(const IPV4& toCompare)
{
    return  this->_address == toCompare._address;
}

uint32_t network::IPV4::Get()
{
    return this->_address;
}

std::string network::IPV4::ToString()
{
    struct in_addr paddr;
    paddr.s_addr = ntohl(this->_address);

    return inet_ntoa(paddr);
}

void network::IPV4::IPV4Validation(const std::string& addr)
{
    if(addr.size() > 15 or std::count(addr.begin(), addr.end(), '.') != 3)
    {
        // TODO: Incluir na classe LOG
        throw std::invalid_argument("Argumento Invalido");
    }
}

void network::IPV4::GetIPV4(std::string interface)
{
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;

    if(getifaddrs(&ifAddrStruct) == -1)
    {
        // TODO: Regitrar na classe LOG
        throw std::runtime_error("Falha ao executar getifaddrs. Erro: " + std::string(std::strerror(errno)));    
    }

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) 
    {
        if (!ifa->ifa_addr)
            continue;

        else if(ifa->ifa_addr->sa_family == AF_INET)
        {
            if(std::string(ifa->ifa_name).find("eth") != std::string::npos or std::string(ifa->ifa_name).find("enp") != std::string::npos)
            {
                void *tempAddrPrt = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            
                char addressBuffer[INET_ADDRSTRLEN];
                
                inet_ntop(AF_INET, tempAddrPrt, addressBuffer, INET_ADDRSTRLEN);
                
                *this = IPV4(addressBuffer);
                
                break;
            }
        }
    }

    if (ifAddrStruct!=NULL) 
        freeifaddrs(ifAddrStruct);
}
