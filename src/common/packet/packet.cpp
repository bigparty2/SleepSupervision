#include "packet.hpp"

using namespace ss::network;

packet::packet(_packet packet)
{
    this->SetPacket(packet);
}

packet::packet(computer computer, packetMesg message, uint16_t port, byte seq)
{
    this->SetPacket(computer, message, port, seq);
}

void packet::SetPacket(computer computer, packetMesg message, uint16_t port, byte seq)
{
    if(this->dataInicialized)
    {
        //TODO: Incluir erro na classe LOG
        throw std::runtime_error("Packet já inicializado");    
    }

    this->dataInicialized = true;
    this->packetData.seqNum = seq;
    // std::memcpy(this->packetData.macOrigin, computer.GetMAC().Get(), 6);
    // this->packetData.ipv4Origin = computer.GetIPV4().Get();
    // std::memcpy(this->packetData.nameOrigin, computer.GetName().c_str(), 64);
    this->packetData.pcOrigin = computer.ToComputerData();
    this->packetData.portOrigin = port;
    this->packetData.timestamp = this->GetTimestamp();
    this->packetData.message = (byte)message;
}

void packet::SetPacket(_packet packet)
{
    if(this->dataInicialized)
    {
        //TODO: Incluir erro na classe LOG
        throw std::runtime_error("Packet já inicializado");    
    }

    this->dataInicialized = true;
    this->packetData = packet;
}

packet::~packet()
{
}

ss::byte* packet::GetPacketData()
{
    this->packetPointerData = new byte[this->GetPacketSize()];

    std::memcpy(this->packetPointerData, &this->packetData, this->GetPacketSize());

    return this->packetPointerData;
}

packet::_packet packet::GetPacket()
{
    return this->packetData;
}

size_t packet::GetPacketSize()
{
    return sizeof(_packet);
}

int64_t packet::GetTimestamp()
{
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
}

bool packet::IsDataInicialized()
{
    return this->dataInicialized;
}