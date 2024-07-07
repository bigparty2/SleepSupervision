
#include "manager.hpp"

using namespace ss;

manager::computersManager::computersManager(bool isHost)
{
    //inicialização dos ponteiros para memória compartilhada
    this->saLastUpdate = mmap(NULL, sizeof(uint64_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    this->saHostname = mmap(NULL, sizeof(char)*64, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    this->saMAC = mmap(NULL, sizeof(char)*6, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    this->saIPV4 = mmap(NULL, sizeof(int32_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    this->saStatus = mmap(NULL, sizeof(uint8_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    this->saIPCControl = mmap(NULL, sizeof(uint8_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    this->saIsHostSeted = mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    //atribuição da variavel host
    this->isHost = isHost;

    //ultima atualização
    *(uint64_t*)this->saLastUpdate = 0;

    //captura dos dados do computador local
    this->thisComputer.GetComputerInfo();

    //atribuição do host caso seja host
    if(this->isHost)
    {
        this->hostComputer = new computer(this->thisComputer);
        *(bool*)this->saIsHostSeted = true; 
    }
    else
    {
        this->hostComputer = nullptr;
        *(bool*)this->saIsHostSeted = false;
    }

    //inicializacao do controle comunicação entre processos
    *(uint64_t*)this->saIPCControl = WAIT;

    //sem_close(sem);
    sem_unlink(SEM_NAME);

    //inicialização do semaforo (semaforo nomeado) (apenas 1 por vez)
    this->sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);

    //Verificação de erro na abertura do semaforo
    if(this->sem == SEM_FAILED)
    {
        // TODO: Incluir erro na classe LOG
        // TODO: Adaptar erro para throw
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
}

manager::computersManager::~computersManager()
{
    sem_close(sem);
    sem_unlink(SEM_NAME);
    if(this->hostComputer != nullptr)
        delete this->hostComputer;
}

uint64_t manager::computersManager::LastUpdate() const
{
    return *(uint64_t*)this->saLastUpdate;
}

void manager::computersManager::UpdateLastUpdate()
{
    *(uint64_t*)this->saLastUpdate += 1;
    //(*(uint64_t*)this->saLastUpdate)++;
}

computers manager::computersManager::Get() const
{
    auto pcList = computers();

    sem_wait(this->sem);

    while((*(uint8_t*)this->saIPCControl) != READY);

    *(uint8_t*)this->saIPCControl = GET;

    while((*(uint8_t*)this->saIPCControl) == GET);

    while((*(uint8_t*)this->saIPCControl) != ENDLIST)
    {
        while((*(uint8_t*)this->saIPCControl) != WAIT);

        pcList.push_back(ReadFromSA());

        *(uint8_t*)this->saIPCControl = NEXT;

        while((*(uint8_t*)this->saIPCControl) == NEXT);
    }

    *(uint8_t*)this->saIPCControl = END;

    sem_post(this->sem);

    return pcList;
}

void manager::computersManager::GetResponse()
{
    for(auto &pcd : this->_data)
    {
        WriteOnSA(pcd);

        *(uint8_t*)this->saIPCControl = WAIT;

        while((*(uint8_t*)this->saIPCControl) != NEXT);
    }

    *(uint8_t*)this->saIPCControl = ENDLIST;

    while((*(uint8_t*)this->saIPCControl) != END);
}

uint64_t manager::computersManager::IndexOf(std::string hostname)
{
    for (int index = 0; index < this->_data.size(); index++)
    {
        if(this->_data.at(index).GetName() == hostname)
            return index;
    }

    return manager::computersManager::npos;
}

uint64_t manager::computersManager::IndexOf(network::IPV4 ipv4)
{
    for (int index = 0; index < this->_data.size(); index++)
    {
        if(this->_data.at(index).GetIPV4() == ipv4)
            return index;
    }

    return manager::computersManager::npos;
}

uint64_t manager::computersManager::IndexOf(network::MAC macAddr)
{
    for (int index = 0; index < this->_data.size(); index++)
    {
        if(this->_data.at(index).GetMAC() == macAddr)
            return index;
    }

    return manager::computersManager::npos;
}

// bool manager::computersManager::Remove(network::MAC macAddr)
// {
//     auto index = manager::computersManager::IndexOf(macAddr);

//     if(index == manager::computersManager::npos)
//         return false;

//     manager::computersManager::RemoveByIndex(index);

//     manager::computersManager::UpdateLastUpdate();

//     return true;
// }

// bool manager::computersManager::Remove(network::IPV4 ipv4)
// {
//     auto index = manager::computersManager::IndexOf(ipv4);

//     if(index == manager::computersManager::npos)
//         return false;

//     manager::computersManager::RemoveByIndex(index);

//     manager::computersManager::UpdateLastUpdate();

//     return true;
// }

// bool manager::computersManager::Remove(std::string hostname)
// {
//     auto index = manager::computersManager::IndexOf(hostname);

//     if(index == manager::computersManager::npos)
//         return false;

//     manager::computersManager::RemoveByIndex(index);

//     manager::computersManager::UpdateLastUpdate();

//     return true;
// }

// void manager::computersManager::RemoveByIndex(uint64_t index)
// {
//     this->_data.erase(this->_data.begin() + index);
// }

void manager::computersManager::Update(computer computer)
{
    sem_wait(this->sem);

    while((*(uint8_t*)this->saIPCControl) != READY);

    *(uint8_t*)this->saIPCControl = UPDATE;

    while((*(uint8_t*)this->saIPCControl) != WAIT);

    WriteOnSA(computer);

    *(uint8_t*)this->saIPCControl = END;

    sem_post(this->sem);
}

void ss::manager::computersManager::Remove(computer computer)
{
    //Se for host, remove o computador informado
    if(this->isHost)
    {
        logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"Remoção de participante como host");

        sem_wait(this->sem);

        while((*(uint8_t*)this->saIPCControl) != READY);

        *(uint8_t*)this->saIPCControl = REMOVE;

        while((*(uint8_t*)this->saIPCControl) != WAIT);

        WriteOnSA(computer);

        *(uint8_t*)this->saIPCControl = END;

        sem_post(this->sem);
    }
    //Se não for host, envia a solicitação para o host para remover o computador informado
    else
    {
        logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"Remoção de participante como participante");

        this->SendExitMessage(computer);
    }
}

void ss::manager::computersManager::RemoveResponse()
{
    *(uint8_t*)this->saIPCControl = WAIT;

    while((*(uint8_t*)this->saIPCControl) != END);

    computer pcd = ReadFromSA();

    auto index = manager::computersManager::IndexOf(pcd.GetIPV4());

    if(index == manager::computersManager::npos)
    {
        logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"Participante não encontrado para remoção");
        return;    
    }

    this->_data.erase(this->_data.begin() + index);

    this->UpdateLastUpdate();

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"Participante removido do sistema");
}

void ss::manager::computersManager::SendExitMessage(computer computer)
{
    auto socket = network::Socket(IPPROTO_UDP);
    
    timeval timeout = {.tv_sec = 5 };
    socket.SetConfig(SO_RCVTIMEO, timeout); //Timeout de recebimento

    uint16_t sequence = 0;

    //TODO: Criar loop para bind em uma porta mesmo se todas estiverem ocupadas, neste caso aguardar liberar uma porta
    uint16_t port = MANAGER_PORT_CLIENT_INIT;
    socket.Bind(port, MANAGER_PORT_CLIENT_END);

    network::packet packet(computer, network::packet::EXIT, port, sequence);

    auto host = this->GetHost();

    auto imLeft = false;

    while(!imLeft)
    {
        logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"Enviando mensagem de saída");
    
        socket.Send(packet, DISCOVERY_PORT_SERVER, host.GetIPV4().Get());

        auto response = socket.receivePacket();

        if(response.IsDataInicialized())
        {
            if(response.GetPacket().message == network::packet::OK and response.GetPacket().seqNum == sequence + 1)
            {
                logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"Mensagem de saída respondida");

                imLeft = true;
            }
            else
            {
                logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"Mensagem de saída não é esperada");
            }
        }
        else
        {
            logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"Mensagem de saída não respondida");
        }
    }
}

void manager::computersManager::UpdateResponse()
{
    *(uint8_t*)this->saIPCControl = WAIT;

    while((*(uint8_t*)this->saIPCControl) != END);

    computer pcd = ReadFromSA();

    auto index = manager::computersManager::IndexOf(pcd.GetIPV4());

    if(index == manager::computersManager::npos)
        return;

    this->_data.at(index) = pcd;

    this->UpdateLastUpdate();
}

void manager::computersManager::Insert(computer computer)
{
    sem_wait(this->sem);

    while((*(uint8_t*)this->saIPCControl) != READY);

    *(uint8_t*)this->saIPCControl = INSERT;

    while((*(uint8_t*)this->saIPCControl) != WAIT);

    WriteOnSA(computer);

    *(uint8_t*)this->saIPCControl = END;

    sem_post(this->sem);
}

void manager::computersManager::InsertResponse()
{
    *(uint8_t*)this->saIPCControl = WAIT;

    while((*(uint8_t*)this->saIPCControl) != END);

    computer pcd = ReadFromSA();

    if(manager::computersManager::IndexOf(pcd.GetIPV4()) != manager::computersManager::npos)
    {
        //TODO: Implementar retorno para indicar que já existe na lista
        logger::GetInstance().Log(__PRETTY_FUNCTION__, "Computador já registrado no sistema");
        return;
    }

    this->_data.push_back(pcd);

    this->UpdateLastUpdate();

    logger::GetInstance().Log(__PRETTY_FUNCTION__, "Computador registrado no sistema");
}

void manager::computersManager::HandleRequest()
{
    auto currentStatus = *(uint8_t*)this->saIPCControl;

    switch (currentStatus)
    {
    case INSERT:

        logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Inserindo computador");
        this->InsertResponse();
        break;

    case UPDATE:

        logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Atualizando computador");
        this->UpdateResponse();
        break;
        
    case GET:

        logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Pegar lista computadores");
        this->GetResponse();
        break;

    case GETHOST:

        logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Pegar computador host");
        this->GetHostResponse();
        break;

    case SETHOST:

        logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Definir computador host");
        this->SetHostResponse();
        break;

    case REMOVE:

        logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Removendo computador");
        this->RemoveResponse();
        break;

    default:
        break;
    }

    *(uint8_t*)this->saIPCControl = READY;
}

computer manager::computersManager::ReadFromSA() const
{
    return computer {
            std::string((char*)this->saHostname),
            network::MAC((byte*)this->saMAC),
            network::IPV4(*(int32_t*)this->saIPV4),
            static_cast<computer::computerStatus>(*(uint8_t*)this->saStatus)
        };
}

void manager::computersManager::WriteOnSA(computer pcd)
{
    //Completa o vetor do hostname com zeros
    std::fill((char*)this->saHostname, (char*)this->saHostname + 64, 0);

    //Copia o hostname para o vetor
    std::memcpy((char*)this->saHostname, pcd.GetName().c_str(), pcd.GetName().size());
    
    //Copia o endereço MAC para o vetor
    std::memcpy((char*)this->saMAC, pcd.GetMAC().Get(), 6);
    
    //Copia o IPV4 para a variavel
    *(int32_t*)this->saIPV4 = pcd.GetIPV4().Get();

    //Copia o Status para a variável
    *(uint8_t*)this->saStatus = static_cast<uint8_t>(pcd.GetStatus());
}

void manager::computersManager::__Insert(computer computer)
{
    this->_data.push_back(computer);
}

computer ss::manager::computersManager::GetHost()
{
    if(hostComputer == nullptr)
    {
        sem_wait(this->sem);

        while((*(uint8_t*)this->saIPCControl) != READY);

        *(uint8_t*)this->saIPCControl = GETHOST;

        while((*(uint8_t*)this->saIPCControl) != WAIT);

        this->hostComputer = new computer(this->ReadFromSA());

        *(uint8_t*)this->saIPCControl = END;

        sem_post(this->sem);

        logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"Host definido como: " + this->hostComputer->GetName() + "|" + this->hostComputer->GetIPV4().ToString());
    }

    return *this->hostComputer;
}

void ss::manager::computersManager::GetHostResponse()
{
    this->WriteOnSA(*this->hostComputer);

    *(uint8_t*)this->saIPCControl = WAIT;

    while((*(uint8_t*)this->saIPCControl) != END);
}

void ss::manager::computersManager::SetHost(computer computer)
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"1 Aguardando liberação semaforo");

    sem_wait(this->sem);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2 Aguardando status READY");
    
    while((*(uint8_t*)this->saIPCControl) != READY);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"3 Definindo status como SETHOST");

    *(uint8_t*)this->saIPCControl = SETHOST;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"4 Aguardando status WAIT");

    while((*(uint8_t*)this->saIPCControl) != WAIT);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"5 Escrevendo PC na memória compartilhada");

    WriteOnSA(computer);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"6 Definindo status como END");

    *(uint8_t*)this->saIPCControl = END;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"7 Liberando semaforo");

    sem_post(this->sem);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"8 Fim");
}

void ss::manager::computersManager::SetHostResponse()
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"1 Definindo status como WAIT");

    *(uint8_t*)this->saIPCControl = WAIT;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2 Aguardando status END");

    while((*(uint8_t*)this->saIPCControl) != END);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"3 Lendo PC da memória compartilhada");

    computer pcd = ReadFromSA();

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"4 Definindo PC host");

    this->hostComputer = new computer(pcd);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"5 Definindo variavel de controle de definição do host como true");

    *(bool*)this->saIsHostSeted = true;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"6 Atualizando contagem de atualização do Manager");

    this->UpdateLastUpdate();

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"7 Fim");
}

bool ss::manager::computersManager::IsHostSeted() const
{
    return *(bool*)this->saIsHostSeted;
}