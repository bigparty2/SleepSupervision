#include "comunicationPacket.hpp"

using namespace ss::network;

ComunicationPacket::~ComunicationPacket()
{
    if(this->packetPointerData != nullptr)
    {
        delete[] this->packetPointerData;
    }
}

ComunicationPacket ComunicationPacket::packetBuilder( 
    computer pcOrigin, uint16_t portOrigin, ComunicationType typeOrigin,  message packetMessage, computer pcDest, 
    uint16_t portDest, ComunicationType typeDest, uint8_t seqNum, computer pcData)
{
    __packetMsg packetMsg;

    packetMsg.seqNum = seqNum;
    packetMsg.originPC = pcOrigin.ToComputerData();
    packetMsg.portOrigin = portOrigin;
    packetMsg.typeOrigin = typeOrigin;
    packetMsg.destPC = pcDest.ToComputerData();
    packetMsg.portDest = portDest;
    packetMsg.typeDest = typeDest;
    packetMsg.msg = packetMessage;
    packetMsg.timestamp = ComunicationPacket::GetTimestamp();
    packetMsg.computerData = pcData.ToComputerData();

    return ComunicationPacket(packetMsg);
}

std::tuple<ss::computer, uint16_t, ComunicationType> ComunicationPacket::GetOriginPCInfo()
{
        return std::make_tuple(computer(packetMsg.originPC), packetMsg.portOrigin, packetMsg.typeOrigin);
}

std::tuple<ss::computer, uint16_t, ComunicationType> ComunicationPacket::GetDestPCInfo()
{
    return std::make_tuple(computer(packetMsg.destPC), packetMsg.portDest, packetMsg.typeDest);
}

ComunicationPacket::message ComunicationPacket::GetPacketMessage()
{
    return packetMsg.msg;
}

ss::computer ComunicationPacket::GetPCData()
{
    return computer(this->packetMsg.computerData);

}

uint8_t ComunicationPacket::GetSeqNum()
{
    return packetMsg.seqNum;
}

size_t ComunicationPacket::GetPacketMSGSize()
{
    return sizeof(__packetMsg);
}

int64_t ComunicationPacket::GetTimestamp()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

ComunicationPacket::__packetMsg ComunicationPacket::GetPacketMSG()
{
    return packetMsg;
}

ss::byte* ComunicationPacket::GetPacketData()
{
    if(this->packetPointerData != nullptr)
    {
        delete[] this->packetPointerData;
        this->packetPointerData = nullptr;
    }

    this->packetPointerData = new byte[this->GetPacketMSGSize()];

    std::memcpy(this->packetPointerData, &this->packetMsg, this->GetPacketMSGSize());
  
    return this->packetPointerData;
}
