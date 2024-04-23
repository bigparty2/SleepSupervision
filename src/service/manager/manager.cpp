
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
    if(isHost)
    {
        this->hostComputer = new computer(this->thisComputer);
        *(bool*)this->saIsHostSeted = true;
        this->isHost = true;    
    }
    else
    {
        this->hostComputer = nullptr;
        this->isHost = false;
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
        return;

    this->_data.erase(this->_data.begin() + index);

    this->UpdateLastUpdate();
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
        socket.Send(packet, DISCOVERY_PORT_SERVER, host.GetIPV4().Get());

        auto response = socket.receivePacket();

        if(response.IsDataInicialized())
        {
            if(response.GetPacket().message == network::packet::OK and response.GetPacket().seqNum == sequence + 1)
            {
                imLeft = true;
            }
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
        //TODO: Implementar retorno para indicar que já existe na lista
        return;

    this->_data.push_back(pcd);

    this->UpdateLastUpdate();
}

void manager::computersManager::HandleRequest()
{
    auto currentStatus = *(uint8_t*)this->saIPCControl;

    switch (currentStatus)
    {
    case INSERT:

        logger::GetInstance().Log(__PRETTY_FUNCTION__, "Inserindo computador");
        this->InsertResponse();
        break;

    case UPDATE:

        logger::GetInstance().Log(__PRETTY_FUNCTION__, "Atualizando computador");
        this->UpdateResponse();
        break;
        
    case GET:

        logger::GetInstance().Log(__PRETTY_FUNCTION__, "Pegar lista computadores");
        this->GetResponse();
        break;

    case GETHOST:

        logger::GetInstance().Log(__PRETTY_FUNCTION__, "Pegar computador host");
        this->GetHostResponse();
        break;

    case SETHOST:

        logger::GetInstance().Log(__PRETTY_FUNCTION__, "Definir computador host");
        this->SetHostResponse();
        break;

    case REMOVE:

        logger::GetInstance().Log(__PRETTY_FUNCTION__, "Removendo computador");
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

        this->hostComputer = new computer(this->thisComputer);

        *(uint8_t*)this->saIPCControl = END;

        sem_post(this->sem);
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
    sem_wait(this->sem);
    
    while((*(uint8_t*)this->saIPCControl) != READY);

    *(uint8_t*)this->saIPCControl = SETHOST;

    while((*(uint8_t*)this->saIPCControl) != WAIT);

    WriteOnSA(computer);

    *(uint8_t*)this->saIPCControl = END;

    sem_post(this->sem);
}

void ss::manager::computersManager::SetHostResponse()
{
    *(uint8_t*)this->saIPCControl = WAIT;

    while((*(uint8_t*)this->saIPCControl) != END);

    computer pcd = ReadFromSA();

    this->hostComputer = new computer(pcd);

    *(bool*)this->saIsHostSeted = true;

    this->UpdateLastUpdate();
}

bool ss::manager::computersManager::IsHostSeted() const
{
    return *(bool*)this->saIsHostSeted;
}