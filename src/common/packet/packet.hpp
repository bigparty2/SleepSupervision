#ifndef PACKET_HPP
#define PACKET_HPP

#include "../types.hpp"
#include "../computer/computer.hpp"
#include <cstdint>

namespace ss
{
    namespace network
    {
        class packet
        {
            public:

            struct _packet
            {
                byte seqNum;
                byte macOrigin[6];
                int32_t ipv4Origin;
                byte nameOrigin[64];
                uint16_t portOrigin;
                int64_t timestamp;
                byte message;
            };

            enum packetMesg
            {
                REGITRY = 0,
                EXIT = 1,
                OK = 2,
                ERROR = 3
            };

            packet(){};
            packet(_packet packet);
            packet(computer computer, packetMesg message, uint16_t port, byte seq = 1);
            ~packet();

            byte* GetPacketData();
            _packet GetPacket();
            static size_t GetPacketSize();
            void SetPacket(computer computer, packetMesg message, uint16_t port, byte seq = 1);
            void SetPacket(_packet packet);

            bool IsDataInicialized();

            private:

            bool dataInicialized = false;

            _packet packetData;

            byte* packetPointerData; 

            int64_t GetTimestamp();
        };
    }
}   

#endif //PACKET_HPP