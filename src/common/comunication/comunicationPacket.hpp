#ifndef COMUNICATIONPACKET_HPP
#define COMUNICATIONPACKET_HPP

#include <cstdint>
#include <tuple>
#include "../computer/computer.hpp"
#include "comunicationType.hpp"

namespace ss
{
    namespace network
    {
        class ComunicationPacket
        {
        public:

            enum message
            {
                REGITRY = 0,
                EXIT = 1,
                OK = 2,
                ERROR = 3,
                ISAWAKE = 4,
                IMAWAKE = 5,
                EMPTY = 6,
                PC_LIST_INIT = 7,
                PC_LIST_NEXT = 8,
                PC_LIST_END = 9,
                PC_LIST_INIT_END = 10
            };

            struct __packetMsg
            {
                uint8_t seqNum;
                computer::computerData originPC;
                uint16_t portOrigin;
                ComunicationType typeOrigin;
                computer::computerData destPC;
                uint16_t portDest;
                ComunicationType typeDest;
                message msg;
                int64_t timestamp;
                computer::computerData computerData;
            };

            ComunicationPacket() : dataInicialized(false) {};

            ComunicationPacket(__packetMsg packet) : packetMsg(packet), dataInicialized(true) {};

            ~ComunicationPacket();

            static ComunicationPacket packetBuilder(computer pcOrigin, uint16_t portOrigin, 
                                                    ComunicationType typeOrigin,  message packetMessage, 
                                                    computer pcDest, uint16_t portDest, ComunicationType typeDest,
                                                    uint8_t seqNum, computer pcData = computer());

            std::tuple<computer, uint16_t, ComunicationType> GetOriginPCInfo();

            std::tuple<computer, uint16_t, ComunicationType> GetDestPCInfo();

            message GetPacketMessage();

            computer GetPCData();

            uint8_t GetSeqNum();

            static size_t GetPacketMSGSize();

            ss::byte* GetPacketData();

            static int64_t GetTimestamp();

            __packetMsg GetPacketMSG();

        private:

            bool dataInicialized;

            __packetMsg packetMsg;

            ss::byte* packetPointerData = nullptr;
        };
    }
}

#endif //COMUNICATIONPACKET_HPP