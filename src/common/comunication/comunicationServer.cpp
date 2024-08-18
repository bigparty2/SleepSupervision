#include "comunication.hpp"

using namespace ss::network;

Comunication<ComunicationType::server>& Comunication<ComunicationType::server>::GetInstance(manager::computersManager *cm)
{
    static Comunication instance;

    if(cm != nullptr)
    {
        instance.cm = cm;
    }

    return instance;
}

Comunication<ComunicationType::server>& Comunication<ComunicationType::server>::operator=(const Comunication<ComunicationType::server>&) 
{
    return GetInstance();
}

void Comunication<ComunicationType::server>::StartAsync()
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Starting server comunication");
    this->mainThread = std::thread(&Comunication<ComunicationType::server>::HandleRequest, this);
    this->listenerThread = std::thread(&Comunication<ComunicationType::server>::HandleListener, this);
    this->senderThread = std::thread(&Comunication<ComunicationType::server>::HandleSander, this);
}

void Comunication<ComunicationType::server>::Stop()
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Stopping server comunication");
    this->SetCommand(Comunication<ComunicationType::server>::Command::STOP);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Joining threads");
    this->mainThread.join();
    this->listenerThread.join();
    this->senderThread.join();
    logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Threads joined");
}

void Comunication<ComunicationType::server>::SendToQueue(computer pcDest, ComunicationType typeDest, ComunicationPacket::message msg, ComunicationType typeOrigin, computer pcData, uint8_t seqNum)
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Sending packet to queue");

    this->LockSem();

    while(GetCommand() != Command::CAN_ACCEPT_NEW_COMMAND);

    SetCommand(Command::ADD_TO_QUEUE);

    while(GetCommand() != Command::CAN_WRITE);

    this->writeSA(ComunicationPacket::packetBuilder(cm->thisComputer,
                                                    this->COM_SENDER_PORT,
                                                    typeOrigin,
                                                    msg,
                                                    pcDest,
                                                    this->COM_LISTENER_PORT,
                                                    typeDest,
                                                    seqNum,
                                                    pcData));

    SetCommand(Command::CAN_READ);

    this->UnlockSem();

    logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Packet sent to queue");
}

void Comunication<ComunicationType::server>::AddToQueueResponse()
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Adding packet to queue");

    SetCommand(Command::CAN_WRITE);

    while(GetCommand() != Command::CAN_READ);

    auto packet = ReadSA();

    this->AddToSendQueue(packet);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Packet added to queue");
}

ComunicationPacket Comunication<ComunicationType::server>::ReadFromQueue(ComunicationType toCom, computer fromPC)
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Reading packet from queue");

    this->LockSem();

    while(GetCommand() != Command::CAN_ACCEPT_NEW_COMMAND);

    SetCommand(Command::GET_FROM_QUEUE);

    while(GetCommand()!= Command::CAN_WRITE);

    //Escreve filtros para leitura (procurar do fromPC para toCom)
    this->writeSA(ComunicationPacket::packetBuilder(fromPC,
                                                    0,
                                                    (ComunicationType)0,
                                                    ComunicationPacket::message::EMPTY,
                                                    cm->thisComputer,
                                                    0,
                                                    toCom,
                                                    0,
                                                    computer()));

    SetCommand(Command::CAN_READ);
    
    while(GetCommand() == Command::CAN_READ);

    if(GetCommand() == Command::NOT_FOUND)
    {
        SetCommand(Command::OK);

        this->UnlockSem();

        logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Packet not found");

        throw ComunicationException(ComunicationException::Type::NOT_FOUND);
    }
    else if (GetCommand() == Command::GET_FROM_QUEUE)
    {
        SetCommand(Command::OK);

        auto packet = ReadSA();

        this->UnlockSem();

        logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Packet found");

        return packet;
    }
    else
    {
        SetCommand(Command::OK);

        this->UnlockSem();

        logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Unknown error");

        throw ComunicationException(ComunicationException::Type::UNKNOWN);
    }
}

void Comunication<ComunicationType::server>::GetFromQueueResponse()
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Getting packet from queue");

    SetCommand(Command::CAN_WRITE);

    while(GetCommand() != Command::CAN_READ);

    auto filters = ReadSA();

    try
    {
        auto packet = this->GetFromReceiveQueue(filters.GetPacketMSG().originPC, filters.GetPacketMSG().typeDest);

        writeSA(packet);

        SetCommand(Command::GET_FROM_QUEUE);

        logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Packet found");
    }
    catch(const ComunicationException& e)
    {
        SetCommand(Command::NOT_FOUND);

        logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Packet not found");
    }

    while(GetCommand() != Command::OK);
}

ComunicationPacket Comunication<ComunicationType::server>::GetFromReceiveQueue(computer::computerData fromPC, ComunicationType toCom)
{
    std::lock_guard<std::mutex> lock(this->mtxReceiveQueue);

    for(int i = this->ReceiveQueue.size() - 1; i >= 0; i--)
    {
        auto packet = this->ReceiveQueue[i];

        if(packet.GetPacketMSG().originPC.ipv4 == fromPC.ipv4 && packet.GetPacketMSG().typeDest == toCom)
        {
            this->ReceiveQueue.erase(this->ReceiveQueue.begin() + i);
            return packet;
        }
    }

    throw ComunicationException(ComunicationException::Type::NOT_FOUND);
}


void Comunication<ComunicationType::server>::ClearMessagesTo(ComunicationType toCom)
{
    this->LockSem();

    while(GetCommand() != Command::CAN_ACCEPT_NEW_COMMAND);

    SetCommand(Command::CLEAR_MESSAGES);

    while(GetCommand() != Command::CAN_WRITE);

    this->writeSA(ComunicationPacket::packetBuilder(computer(),
                                                    0,
                                                    (ComunicationType)0,
                                                    ComunicationPacket::message::EMPTY,
                                                    cm->thisComputer,
                                                    0,
                                                    toCom,
                                                    0,
                                                    computer()));

    SetCommand(Command::CAN_READ);

    while(GetCommand() != Command::OK);

    this->UnlockSem();
}

void Comunication<ComunicationType::server>::ClearMessagesResponse()
{
    SetCommand(Command::CAN_WRITE);

    while(GetCommand() != Command::CAN_READ);

    auto filters = ReadSA();

    std::lock_guard<std::mutex> lock(this->mtxReceiveQueue);

    for(int i = this->ReceiveQueue.size() - 1; i >= 0; i--)
    {
        auto packet = this->ReceiveQueue[i];

        if(packet.GetPacketMSG().typeDest == filters.GetPacketMSG().typeDest)
        {
            this->ReceiveQueue.erase(this->ReceiveQueue.begin() + i);
        }
    }

    SetCommand(Command::OK);
}

Comunication<ComunicationType::server>::Comunication()
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Starting server comunication");
    logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Init constructor");

    //inicialização memória compartilhada
    this->ComunicationServerCommandSA = mmap(NULL, sizeof(Command), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    this->PacketMsgSA = mmap(NULL, sizeof(ComunicationPacket::__packetMsg), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    //inicialização semáforo
    sem_unlink(SEM_NAME.c_str());
    this->sem = sem_open(SEM_NAME.c_str(), O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);

    //Inicialização do comando
    SetCommand(Command::CAN_ACCEPT_NEW_COMMAND);

    //Inicialização dos sockets
    this->socketSender = new network::Socket(IPPROTO_UDP);
    this->socketSender->SetConfig(SO_REUSEPORT, 1);
    this->socketSender->SetConfig(SO_BROADCAST, 1);
    uint16_t portSender = static_cast<uint16_t>(COM_SENDER_PORT);
    this->socketSender->Bind(portSender);

    this->socketListener = new network::Socket(IPPROTO_UDP);
    timeval listenerTimeout = {.tv_sec = timeout };
    this->socketListener->SetConfig(SO_RCVTIMEO, listenerTimeout);
    this->socketListener->SetConfig(SO_REUSEPORT, 1);
    uint16_t portListener = static_cast<uint16_t>(COM_LISTENER_PORT);
    this->socketListener->Bind(portListener);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__, "End constructor");
}

Comunication<ComunicationType::server>::~Comunication()
{
    //liberação memória compartilhada
    munmap(this->ComunicationServerCommandSA, sizeof(Command));
    munmap(this->PacketMsgSA, sizeof(packet::packetMesg));

    //liberação semáforo
    sem_destroy(this->sem);
    sem_unlink(SEM_NAME.c_str());

    //liberação sockets
    delete this->socketSender;
    delete this->socketListener;
}

void Comunication<ComunicationType::server>::HandleRequest()
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Starting Handling requests");
    while(this->GetCommand() != Command::STOP)
    {
        switch(GetCommand())
        {
            case Command::ADD_TO_QUEUE:
                logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Handling ADD_TO_QUEUE");
                this->AddToQueueResponse();
                break;
            case Command::GET_FROM_QUEUE:
                logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Handling GET_FROM_QUEUE");
                this->GetFromQueueResponse();
                break;
            case Command::CLEAR_MESSAGES:
                logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Handling CLEAR_MESSAGES");
                this->ClearMessagesResponse();
                break;
            default:
                logger::GetInstance().Debug(__PRETTY_FUNCTION__, "No command to handle");
                break;
        }

        SetCommand(Command::CAN_ACCEPT_NEW_COMMAND);

        thread::Sleep(100);
    }
}

void Comunication<ComunicationType::server>::SetCommand(Command command)
{
    *((Command*)this->ComunicationServerCommandSA) = command;
}

Comunication<ComunicationType::server>::Command Comunication<ComunicationType::server>::GetCommand()
{
    return *((Command*)this->ComunicationServerCommandSA);
}

void Comunication<ComunicationType::server>::LockSem()
{
    sem_wait(this->sem);
}

void Comunication<ComunicationType::server>::UnlockSem()
{
    sem_post(this->sem);
}

void Comunication<ComunicationType::server>::writeSA(ComunicationPacket packet)
{
    *((ComunicationPacket::__packetMsg*)this->PacketMsgSA) = packet.GetPacketMSG();
}

ComunicationPacket Comunication<ComunicationType::server>::ReadSA()
{
    return ComunicationPacket(*((ComunicationPacket::__packetMsg*)this->PacketMsgSA));
}

bool Comunication<ComunicationType::server>::IsSemLocked()
{
    int semValue;
    sem_getvalue(this->sem, &semValue);

    return semValue == 0;
}

void Comunication<ComunicationType::server>::HandleListener()
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Starting Listener");
    while(GetCommand() != Command::STOP)
    {
        try
        {
            auto packet = this->socketListener->Receive();

            logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Received packet");

            this->socketListener->Send(this->BuildOKPacket(packet));

            this->AddToReceiveQueue(packet);

        } catch (std::runtime_error &e)
        {
            ss::logger::GetInstance().Debug(__PRETTY_FUNCTION__, e.what());
            continue;
        }
    }
}

void Comunication<ComunicationType::server>::HandleSander()
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Starting Sender");
    while(GetCommand() != Command::STOP)
    {
        if(!this->SendQueue.empty())
        {
            auto packet = this->GetFromSendQueue();

            this->socketSender->Send(packet);

            //Receber confirmação de recebimento
            try
            {
                auto ret = this->socketSender->Receive();
                
                if(ret.GetPacketMessage() == ComunicationPacket::message::OK && packet.GetSeqNum() == ret.GetSeqNum() + 1)
                {
                    continue;
                }
                else
                {
                    //Adiciona mensagem de volta ao final da fila
                    this->AddToSendQueue(packet);

                }
                
            } catch (std::runtime_error &e)
            {
                this->AddToSendQueue(packet);
                ss::logger::GetInstance().Log(__PRETTY_FUNCTION__, e.what());
                continue;
            } 
        }
        else
        {
            logger::GetInstance().Debug(__PRETTY_FUNCTION__, "SendQueue is empty");
            thread::Sleep(100);
        }
    }
}

bool Comunication<ComunicationType::server>::ReceiveQueueContains(ComunicationPacket packet)
{
    for(auto &p : this->ReceiveQueue)
    {
        if(p.GetPacketData() == packet.GetPacketData())
        {
            return true;
        }
    }

    return false;
}

bool Comunication<ComunicationType::server>::AddToSendQueue(ComunicationPacket packet)
{
    std::lock_guard<std::mutex> lock(this->mtxSendQueue);
    this->SendQueue.push_back(packet);
    return true;
}

bool Comunication<ComunicationType::server>::AddToReceiveQueue(ComunicationPacket packet)
{
    std::lock_guard<std::mutex> lock(this->mtxReceiveQueue);
    if(!this->ReceiveQueueContains(packet))
    {
        this->ReceiveQueue.push_back(packet);
        return true;
    }

    return false;
}

ComunicationPacket Comunication<ComunicationType::server>::GetFromSendQueue()
{
    std::lock_guard<std::mutex> lock(this->mtxSendQueue);
    if(this->SendQueue.empty())
        throw ComunicationException(ComunicationException::Type::EMPTY);
    auto packet = this->SendQueue.front();
    this->SendQueue.erase(this->SendQueue.begin());
    return packet;
}

ComunicationPacket Comunication<ComunicationType::server>::GetFromReceiveQueue()
{
    std::lock_guard<std::mutex> lock(this->mtxReceiveQueue);
    if(this->ReceiveQueue.empty())
        throw ComunicationException(ComunicationException::Type::EMPTY);
    auto packet = this->ReceiveQueue.front();
    this->ReceiveQueue.erase(this->ReceiveQueue.begin());
    return packet;
}

ComunicationPacket Comunication<ComunicationType::server>::BuildOKPacket(ComunicationPacket packet)
{
    ComunicationPacket::__packetMsg packetMSG = packet.GetPacketMSG();

    auto destPC = packet.GetOriginPCInfo();
    auto originPC = packet.GetDestPCInfo();

    //Destino vira origem
    packetMSG.originPC = std::get<0>(destPC).ToComputerData();
    packetMSG.portOrigin = std::get<1>(destPC);
    packetMSG.typeOrigin = std::get<2>(destPC);

    //Origem vira destino
    packetMSG.destPC = std::get<0>(originPC).ToComputerData();
    packetMSG.portDest = std::get<1>(originPC);
    packetMSG.typeDest = std::get<2>(originPC);

    packetMSG.msg = ComunicationPacket::message::OK;
    packetMSG.timestamp = ComunicationPacket::GetTimestamp();
    packetMSG.seqNum = packetMSG.seqNum++;    

    return ComunicationPacket(packetMSG);
}


