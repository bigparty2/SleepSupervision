#include "discovery.hpp"

using namespace ss;

discovery::DiscoverySubservice::DiscoverySubservice(manager::computersManager &computersManager)
{
    this->computersManager = &computersManager;

    this->TIMEOUT = {.tv_sec = 1 };
}

discovery::DiscoverySubservice::~DiscoverySubservice()
{
}

void discovery::DiscoverySubservice::Start(bool isServer)
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

void discovery::DiscoverySubservice::Stop()
{
}

void discovery::DiscoverySubservice::clientRun()
{
    //Criacao de socket UDP
    auto socket = network::Socket(IPPROTO_UDP);

    //Configuracao do socket
    // socket.SetConfig(SO_REUSEPORT, 1);      // Enable port reuse
    socket.SetConfig(SO_BROADCAST, 1);      //Habilita broadcast
    socket.SetConfig(SO_RCVTIMEO, TIMEOUT); //Timeout de recebimento

    //TODO: Criar loop para bind em uma porta mesmo se todas estiverem ocupadas
    //Bind do socket em uma porta de descoberta
    uint16_t port = DISCOVERY_PORT_CLIENT_INIT;
    // socket.Bind(port, DISCOVERY_PORT_CLIENT_END);
    socket.Bind(port);

    //Variavel de controle de sequencia de mensagens
    uint16_t sequence = 0;

    //Pacote de descoberta / registro no sistema
    network::packet packet(this->computersManager->thisComputer, network::packet::REGITRY, port, sequence);

    //Variavel de controle para descoberta
    bool discovery = false;

    try
    {
        //Loop de descoberta
        while(discovery != true)
        // while(true)
        {
            logger::GetInstance().Log(__PRETTY_FUNCTION__ ,"Procurando Host.");

            //Envio de pacote de descoberta
            socket.Send(packet, DISCOVERY_PORT_SERVER, INADDR_BROADCAST);

            //Recebimento de pacote de resposta
            auto response = socket.receivePacket();

            if(!response.IsDataInicialized())
            {
                //log
                logger::GetInstance().Log(__PRETTY_FUNCTION__ ,"Host não respondeu.");
                continue;
            }

            //Verifica se o pacote recebido é uma resposta
            if(response.GetPacket().message == network::packet::OK)
            {
                //Computador host
                auto host = computer((char*)response.GetPacket().nameOrigin, 
                                    network::MAC(response.GetPacket().macOrigin), 
                                    network::IPV4(response.GetPacket().ipv4Origin), 
                                    computer::computerStatus::awake);

                //Definição do computador host
                this->computersManager->SetHost(host);

                //log
                logger::GetInstance().Log(__PRETTY_FUNCTION__ ,"Host encontrado: " + host.GetName() + "|" + host.GetIPV4().ToString());
            
                //Computador adicionado no sistema
                discovery = true;
            }
            else
            {
                //log
                logger::GetInstance().Log(__PRETTY_FUNCTION__ ,"Host não encontrado.");
            }        
        }

    }
    catch(const std::exception& e)
    {
        // std::cerr << e.what() << '\n';
        logger::GetInstance().Error(__PRETTY_FUNCTION__ ,e.what());
    }
    

    //log
    logger::GetInstance().Log(__PRETTY_FUNCTION__ ,"Finalizando descoberta[Client].");

}

void discovery::DiscoverySubservice::serverRun()
{
    //Criacao de socket UDP
    auto socket = network::Socket(IPPROTO_UDP);

    //logger::GetInstance().Log(__PRETTY_FUNCTION__ ,"Socket criado");

    //Configuracao do socket
    socket.SetConfig(SO_RCVTIMEO, TIMEOUT); //Timeout de recebimento

    //bind socket na porta do servidor
    auto port = DISCOVERY_PORT_SERVER;
    socket.Bind(port);

    //loop de recebimento de pacotes
    while(true)
    {
        //Recebimento de pacote
        auto packet = socket.receivePacket();

        //Verifica se o pacote é de registro
        if(packet.IsDataInicialized())
        {
            switch (packet.GetPacket().message)
            {
            case network::packet::REGITRY:
            {
                //Adiciona computador no sistema
                this->computersManager->Insert(
                    computer(std::string((char*)packet.GetPacket().nameOrigin), 
                            network::MAC(packet.GetPacket().macOrigin), 
                            network::IPV4(packet.GetPacket().ipv4Origin), 
                            computer::computer::awake)
                    );

                //Pacote de resposta
                network::packet response(this->computersManager->thisComputer, network::packet::OK, port, packet.GetPacket().seqNum + 1);

                //Envio de pacote de resposta
                // socket.Send(response, packet.GetPacket().portOrigin, packet.GetPacket().ipv4Origin);
                socket.Send(response, packet.GetPacket().portOrigin, "192.168.0.255");

                ss::thread::Sleep(125);
                
                break;
            }
            
            default:
                break;
            }
        }
    }
}

