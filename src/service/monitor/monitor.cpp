#include "monitor.hpp"

using namespace ss;

monitor::MonitorSubservice::MonitorSubservice(manager::computersManager& computersManager)
{
    this->computersManager = &computersManager;

    this->TIMEOUT = {.tv_sec = 1 };

    this->UpdateComputersToMonior();
}

monitor::MonitorSubservice::~MonitorSubservice()
{
}

void monitor::MonitorSubservice::Start(bool isServer)
{
    if (isServer)
    {
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
            }
        }
    
        ss::thread::Sleep(500);
    }
}

void monitor::MonitorSubservice::serverRun()
{
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
            this->UpdateComputersToMonior();
        }
        else
        {
            for(auto& computerMonitor : this->failedMonitors)
            {
                auto packet = network::packet(this->computersManager->thisComputer, network::packet::ISAWAKE, port, 0);

                socket.Send(packet, MONITOR_PORT_CLIENT, computerMonitor.Computer.GetIPV4().Get());

                auto packetRec = socket.receivePacket();

                if(packetRec.IsDataInicialized())
                {
                    if(packetRec.GetPacket().message == network::packet::IMAWAKE)
                    {
                        computerMonitor.failCount = 0;

                        if(computerMonitor.Computer.GetStatus() == computer::computerStatus::sleep)
                        {
                            computerMonitor.Computer.SetStatus(computer::computerStatus::awake);
                            this->computersManager->Update(computerMonitor.Computer);
                        }
                    }
                    else
                    {
                        computerMonitor.failCount++;
                    }
                }
                else
                {
                    computerMonitor.failCount++;
                }

                if(computerMonitor.failCount > MAX_FAILS)
                {
                    computerMonitor.Computer.SetStatus(computer::computerStatus::sleep);
                    this->computersManager->Update(computerMonitor.Computer);
                }
            }
        }

        thread::Sleep(500);
    }
}

void monitor::MonitorSubservice::UpdateComputersToMonior()
{
    this->failedMonitors.clear();

    for(auto& computer : this->computersManager->Get())
    {
        if(computer.GetStatus() == computer::computerStatus::sleep)
        {
            this->failedMonitors.push_back({computer, 0});
        }
    }

    this->lastUpdate = this->computersManager->LastUpdate();
}