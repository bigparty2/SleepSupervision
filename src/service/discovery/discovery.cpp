#include "discovery.hpp"

using namespace ss;

discovery::DiscoverySubservice::DiscoverySubservice(manager::computersManager &computersManager)
{
    this->computersManager = &computersManager;

    this->TIMEOUT = {.tv_sec = 5 };
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
    socket.SetConfig(SO_BROADCAST, 1);      //Habilita broadcast
    socket.SetConfig(SO_RCVTIMEO, TIMEOUT); //Timeout de recebimento

    //TODO: Criar loop para bind em uma porta mesmo se todas estiverem ocupadas
    //Bind do socket em uma porta de descoberta
    uint16_t port = DISCOVERY_PORT_CLIENT_INIT;
    socket.Bind(port, DISCOVERY_PORT_CLIENT_END);

    //Variavel de controle de sequencia de mensagens
    uint16_t sequence = 0;

    //Pacote de descoberta / registro no sistema
    network::packet packet(this->computersManager->thisComputer, network::packet::REGITRY, port, sequence);

    //Variavel de controle para descoberta
    bool discovery = false;

    //Loop de descoberta
    while(discovery != true)
    {
        //Envio de pacote de descoberta
        socket.Send(packet, DISCOVERY_PORT_SERVER, INADDR_BROADCAST);

        //Recebimento de pacote de resposta
        auto response = socket.receivePacket();

        if(response.IsDataInicialized())
        {
            //Verifica se o pacote recebido é uma resposta
            if(response.GetPacket().message == network::packet::OK)
            {
                //Computador host
                auto host = computer((char*)response.GetPacket().nameOrigin, 
                                    network::MAC(response.GetPacket().macOrigin), 
                                    network::IPV4(response.GetPacket().ipv4Origin), 
                                    computer::computerStatus::awake);            

                //log
                logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"Host encontrado: " + host.GetName() + "|" + host.GetIPV4().ToString());

                //Definição do computador host
                this->computersManager->SetHost(host);
            
                //Computador adicionado no sistema
                discovery = true;
            }
        }        
    }
}

void discovery::DiscoverySubservice::serverRun()
{
    //Criacao de socket UDP
    auto socket = network::Socket(IPPROTO_UDP);

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
                    auto participant = computer(std::string((char*)packet.GetPacket().nameOrigin), 
                                                network::MAC(packet.GetPacket().macOrigin), 
                                                network::IPV4(packet.GetPacket().ipv4Origin), 
                                                computer::computer::awake);

                    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"Participante ingressando: " + participant.GetName() + "|" + participant.GetIPV4().ToString());

                    //Adiciona computador no sistema
                    this->computersManager->Insert(participant);

                    //Pacote de resposta
                    network::packet response(this->computersManager->thisComputer, network::packet::OK, port, packet.GetPacket().seqNum + 1);

                    //Envio de pacote de resposta
                    socket.Send(response, packet.GetPacket().portOrigin, packet.GetPacket().ipv4Origin);
                    
                    break;
                }

                case network::packet::EXIT:
                {
                    auto participant = computer(std::string((char*)packet.GetPacket().nameOrigin), 
                                                network::MAC(packet.GetPacket().macOrigin), 
                                                network::IPV4(packet.GetPacket().ipv4Origin), 
                                                computer::computer::awake);

                    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"Participante saindo: " + participant.GetName() + "|" + participant.GetIPV4().ToString());

                    //Remove computador do sistema
                    this->computersManager->Remove(participant);

                    //Pacote de resposta
                    network::packet response(this->computersManager->thisComputer, network::packet::OK, port, packet.GetPacket().seqNum + 1);

                    //Envio de pacote de resposta
                    socket.Send(response, packet.GetPacket().portOrigin, packet.GetPacket().ipv4Origin);

                    break;
                }
            
            default:
                break;
            }
        }
    }
}

