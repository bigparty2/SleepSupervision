#include "monitor.hpp"

using namespace ss;

monitor::MonitorSubservice::MonitorSubservice(manager::computersManager& computersManager)
{
    this->computersManager = &computersManager;

    this->TIMEOUT = {.tv_sec = 1 };
}

monitor::MonitorSubservice::~MonitorSubservice()
{
}

void monitor::MonitorSubservice::Start(bool isServer)
{
    if (isServer)
    {
        this->UpdateComputersToMonior();
        serverRun();
    }
    else
    {
        clientRun();
    }
}

void monitor::MonitorSubservice::Stop()
{
}

void monitor::MonitorSubservice::clientRun()
{
    //log
    logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Iniciado");

    // Create a UDP socket
    auto socket = network::Socket(IPPROTO_UDP);

    // Configure the socket
    socket.SetConfig(SO_REUSEPORT, 1);      // Enable port reuse
    socket.SetConfig(SO_RCVTIMEO, TIMEOUT); // Receive timeout

    // Bind the socket to a monitoring port
    uint16_t port = MONITOR_PORT_CLIENT;
    socket.Bind(port);

    // Monitoring client loop
    while(true)
    {
        auto packet = socket.receivePacket();

        if(packet.IsDataInicialized())
        {
            if(packet.GetPacket().message == network::packet::ISAWAKE)
            {
                auto returnPacket = network::packet(this->computersManager->thisComputer, network::packet::IMAWAKE, port, packet.GetPacket().seqNum + 1);

                socket.Send(returnPacket, MONITOR_PORT_SERVER, packet.GetPacket().ipv4Origin);
            
                logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Pacote do tipo ISAWAKE recebido e respondido ao servidor");
            }
        }
    
        ss::thread::Sleep(1000);
    }
}

void monitor::MonitorSubservice::serverRun()
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Iniciado");

    // Create a UDP socket
    auto socket = network::Socket(IPPROTO_UDP);

    // Configure the socket
    socket.SetConfig(SO_RCVTIMEO, TIMEOUT); // Receive timeout

    // Bind the socket to a monitoring port
    uint16_t port = MONITOR_PORT_SERVER;
    socket.Bind(port);

    // Monitoring server loop
    while(true)
    {
        if(this->lastUpdate != this->computersManager->LastUpdate())
        {
            logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Atualização da lista de computadores");

            this->UpdateComputersToMonior();
        }
        else if (this->failedMonitors.size() > 0)
        {
            logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Iniciando verificação dos computadores");

            for(auto& computerMonitor : this->failedMonitors)
            {
                logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Iniciada verificação do computador " + computerMonitor.Computer.GetName());

                auto packet = network::packet(this->computersManager->thisComputer, network::packet::ISAWAKE, port, 0);

                socket.Send(packet, MONITOR_PORT_CLIENT, computerMonitor.Computer.GetIPV4().Get());

                auto packetRec = socket.receivePacket();

                if(packetRec.IsDataInicialized())
                {
                    if(packetRec.GetPacket().message == network::packet::IMAWAKE)
                    {
                        logger::GetInstance().Debug(__PRETTY_FUNCTION__, computerMonitor.Computer.GetName() + " respondeu que está acordado");

                        computerMonitor.failCount = 0;

                        if(computerMonitor.Computer.GetStatus() == computer::computerStatus::sleep)
                        {
                            computerMonitor.Computer.SetStatus(computer::computerStatus::awake);
                            this->computersManager->Update(computerMonitor.Computer);

                            logger::GetInstance().Debug(__PRETTY_FUNCTION__, computerMonitor.Computer.GetName() + " estava com status de 'dormindo'");
                        }
                    }
                    else
                    {
                        computerMonitor.failCount++;
                        logger::GetInstance().Debug(__PRETTY_FUNCTION__, computerMonitor.Computer.GetName() + " não respondeu a solicitacao. " + std::to_string(computerMonitor.failCount) + " falhas.");
                    }
                }
                else if (computerMonitor.Computer.GetStatus() == computer::computerStatus::awake)
                {
                    computerMonitor.failCount++;
                    logger::GetInstance().Debug(__PRETTY_FUNCTION__, computerMonitor.Computer.GetName() + " não respondeu a solicitacao. " + std::to_string(computerMonitor.failCount) + " falhas.");
                }

                if(computerMonitor.Computer.GetStatus() == computer::computerStatus::awake && computerMonitor.failCount > MAX_FAILS)
                {
                    computerMonitor.Computer.SetStatus(computer::computerStatus::sleep);
                    this->computersManager->Update(computerMonitor.Computer);
                    logger::GetInstance().Log(__PRETTY_FUNCTION__, computerMonitor.Computer.GetName() + " ultrapassou o numero maximo de falhas e foi considerado como dormindo");
                }
            }
        }

        thread::Sleep(500);
    }
}

void monitor::MonitorSubservice::UpdateComputersToMonior()
{
    this->failedMonitors.clear();

    auto computersList = this->computersManager->Get();

    for(auto& computer : computersList)
    {
        this->failedMonitors.push_back({computer, 0});  
    }

    this->lastUpdate = this->computersManager->LastUpdate();
}