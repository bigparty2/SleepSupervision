#ifndef COMUNICATION_HPP
#define COMUNICATION_HPP

#include <cstddef>
#include <vector>
#include <thread>
#include <tuple>
#include <mutex>
#include "../socket/socket.hpp"
#include "../../service/manager/manager.hpp"
#include "../logger/logger.hpp"
#include <semaphore.h>
#include "../thread/thread.hpp"
#include <sys/mman.h>

#include "comunicationType.hpp"
#include "comunicationPacket.hpp"


namespace ss
{
    namespace network
    {
        template<ComunicationType type>
        class Comunication{};

        template<>
        class Comunication<ComunicationType::monitorServer>
        {
        public:

            bool IsAwake(computer& toPC);

        private:

            short timeout = 1;

            bool WaitIsAwakeResponse(computer& toPC, short timeout);
        };

        template<>
        class Comunication<ComunicationType::monitorClient>
        {
        public:

            bool ResponseIsAwakeRequest();

        private:

            short timeout = 1;

            bool ResponseIsAwakeRequestResponse(computer& toPC, short timeout);
        };
        
        template<>
        class Comunication<ComunicationType::discoveryServer>
        {
        public:

            computer* LookForComputer();
        };

        template<>
        class Comunication<ComunicationType::discoveryClient>
        {
        public:

            computer* LookForHost();
        };

        template<>
        class Comunication<ComunicationType::managerServer>
        {
        public:

            void SendComputerList();
        };

        template<>
        class Comunication<ComunicationType::managerClient>
        {
        public:

            std::vector<computer> GetComputerList();
        };

        template<>
        class Comunication<ComunicationType::bully>
        {
        public:

            /**
             * @brief Inicia uma nova eleição (algoritmo do valentão).
             * 
             * @param requesterID O ID do processo que iniciou a eleição.
             * 
             * @return O ID do processo eleito.
             */
            size_t NewElection(size_t requesterID);            
        };

        template<>
        class Comunication<ComunicationType::server>
        {
        public:

            

            static Comunication& GetInstance(manager::computersManager *cm = nullptr);
            
            Comunication& operator=(const Comunication&);

            void StartAsync();

            void Stop();

            void SendToQueue(computer pcDest, ComunicationType typeDest, ComunicationPacket::message msg, ComunicationType typeOrigin, computer pcData = computer(), uint8_t seqNum = 0);

            ComunicationPacket ReadFromQueue(ComunicationType toCom, computer fromPC = computer());

            void ClearMessagesTo(ComunicationType toCom);

        private:

            enum Command
            {
                CAN_ACCEPT_NEW_COMMAND,
                ADD_TO_QUEUE,
                GET_FROM_QUEUE,
                CAN_READ,
                CAN_WRITE,
                STOP,
                NOT_FOUND,
                OK,
                CLEAR_MESSAGES
            };

            Comunication(const Comunication&) = delete;
            Comunication();
            ~Comunication();

            void HandleRequest();

            void AddToQueueResponse();

            void GetFromQueueResponse();

            void ClearMessagesResponse();

            void HandleSander();

            void HandleListener();

            void RemoveOldMessages(int limitTime);

            void SetCommand(Command command);

            Command GetCommand();

            ComunicationPacket BuildOKPacket(ComunicationPacket toPC); 

            Socket* socketSender;
            
            Socket* socketListener;

            std::thread mainThread;

            std::thread listenerThread;

            std::thread senderThread;

            std::vector<ComunicationPacket> SendQueue;

            std::vector<ComunicationPacket> ReceiveQueue;

            bool ReceiveQueueContains(ComunicationPacket packet);

            bool AddToSendQueue(ComunicationPacket packet);

            ComunicationPacket GetFromSendQueue();

            bool AddToReceiveQueue(ComunicationPacket packet);

            ComunicationPacket GetFromReceiveQueue();
            ComunicationPacket GetFromReceiveQueue(computer::computerData fromPC, ComunicationType toCom);

            std::mutex mtxSendQueue;
            std::mutex mtxReceiveQueue;

            void* ComunicationServerCommandSA;

            bool IsSemLocked();

            void LockSem();

            void UnlockSem();

            void writeSA(ComunicationPacket);
            ComunicationPacket ReadSA();

            manager::computersManager *cm = nullptr;

            short timeout = 1;
            int timeToConsiderOldMessageInSeccounds = 20;

            void* PacketMsgSA;

            sem_t* sem = nullptr;

            const std::string SEM_NAME = "/SS_COMUNICATION_SEM";

            const uint16_t COM_LISTENER_PORT = 49999;
            const uint16_t COM_SENDER_PORT = 49998;
        };

        class ComunicationException : public std::exception
        {
        public:

            enum class Type
            {
                NOT_FOUND,
                UNKNOWN,
                EMPTY
            };

            const Type type;

            const std::string message;

            const char* what() const noexcept override { return message.c_str(); }

            ComunicationException(Type type, const std::string& message = "Comunication error") : message(message), type(type) {}
        };
    }
}

#endif // COMUNICATION_HPP