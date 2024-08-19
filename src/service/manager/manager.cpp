
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
    this->saComputerData = mmap(NULL, sizeof(computer::computerData), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    //atribuição da variavel host
    this->isHost = isHost;

    //ultima atualização
    *(uint64_t*)this->saLastUpdate = 0;

    //captura dos dados do computador local
    this->thisComputer.GetComputerInfo();

    //atribuição do host caso seja host
    if(this->isHost)
    {
        this->thisComputer.SetID(this->GetNewID());
        this->thisComputer.SetLeader();
        this->_data.push_back(this->thisComputer);
        this->hostComputer = new computer(this->thisComputer);
        *(bool*)this->saIsHostSeted = true; 
    }
    else
    {
        this->thisComputer.SetParticipant();
        this->hostComputer = nullptr;
        *(bool*)this->saIsHostSeted = false;
        // this->pcListUpdateThreadListener = std::thread(&computersManager::PCListUpdateListener, this);
        this->pcListUpdateThreadListener = std::thread([this]() { this->ListenPCListUpdate(); });
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
    //Dipara em brodcast a informação de que a lista foi atualizada
    // auto socket = network::Socket(IPPROTO_UDP);
    // socket.SetConfig(SO_BROADCAST, 1);      //Habilita broadcast
    // network::packet packet(this->thisComputer, network::packet::LISTUPDATE, computersManager::MANAGER_LEADER_PORT, 0);
    // socket.Send(packet, computersManager::PCLIST_UPDATE_PORT, INADDR_BROADCAST);

    *(uint64_t*)this->saLastUpdate += 1;

    if(isHost)
    {
        this->SendPCListUpdate();
    }
    
    //(*(uint64_t*)this->saLastUpdate)++;
}

computers manager::computersManager::Get() const
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"1. Iniciando processo de obtenção de lista de computadores | saIPCControl: " + this->IPCControlToString());

    auto pcList = computers();

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2. Aguardando liberação semaforo | saIPCControl: " + this->IPCControlToString());

    sem_wait(this->sem);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"3. Aguardando status READY | saIPCControl: " + this->IPCControlToString());

    while((*(uint8_t*)this->saIPCControl) != READY);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"4. Definindo status como GET | saIPCControl: " + this->IPCControlToString());

    *(uint8_t*)this->saIPCControl = GET;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"5. Aguardando status WAIT | saIPCControl: " + this->IPCControlToString());

    while((*(uint8_t*)this->saIPCControl) == GET);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"6. Iniciando processo de leitura de computadores | saIPCControl: " + this->IPCControlToString());

    while((*(uint8_t*)this->saIPCControl) != ENDLIST)
    {
        while((*(uint8_t*)this->saIPCControl) != WAIT);

        pcList.push_back(ReadFromSA());

        *(uint8_t*)this->saIPCControl = NEXT;

        while((*(uint8_t*)this->saIPCControl) == NEXT);
    }

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"7. Definindo status como END | saIPCControl: " + this->IPCControlToString());

    *(uint8_t*)this->saIPCControl = END;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"8. Liberando semaforo | saIPCControl: " + this->IPCControlToString());

    sem_post(this->sem);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"9. Finalizando processo de obtenção de lista de computadores | saIPCControl: " + this->IPCControlToString());

    return pcList;
}

void manager::computersManager::GetResponse()
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"1. Iniciando processo de resposta de obtenção de lista de computadores | saIPCControl: " + this->IPCControlToString());

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2. Iniciando escrita dos computadores na área de memória compartilhada | saIPCControl: " + this->IPCControlToString());

    for(auto &pcd : this->_data)
    {
        WriteOnSA(pcd);

        *(uint8_t*)this->saIPCControl = WAIT;

        while((*(uint8_t*)this->saIPCControl) != NEXT);
    }

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"3. Definindo status como ENDLIST | saIPCControl: " + this->IPCControlToString());

    *(uint8_t*)this->saIPCControl = ENDLIST;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"4. Aguardando status END | saIPCControl: " + this->IPCControlToString());

    while((*(uint8_t*)this->saIPCControl) != END);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"5. Finalizando processo de resposta de obtenção de lista de computadores | saIPCControl: " + this->IPCControlToString());
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

void manager::computersManager::Update(computer computer)
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"1. Iniciando processo de atualização de computador | saIPCControl: " + this->IPCControlToString());

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2. Aguardando liberação de semaforo | saIPCControl: " + this->IPCControlToString());

    sem_wait(this->sem);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"3. Aguardando status READY | saIPCControl: " + this->IPCControlToString());

    while((*(uint8_t*)this->saIPCControl) != READY);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"4. Definindo status como UPDATE | saIPCControl: " + this->IPCControlToString());

    *(uint8_t*)this->saIPCControl = UPDATE;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"5. Aguardando status WAIT | saIPCControl: " + this->IPCControlToString());

    while((*(uint8_t*)this->saIPCControl) != WAIT);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"6. Escrevendo PC na memória compartilhada | saIPCControl: " + this->IPCControlToString());

    WriteOnSA(computer);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"7. Definindo status como END | saIPCControl: " + this->IPCControlToString());

    *(uint8_t*)this->saIPCControl = END;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"8. Liberando semaforo | saIPCControl: " + this->IPCControlToString());

    sem_post(this->sem);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"9. Finalizando processo de atualização de computador | saIPCControl: " + this->IPCControlToString());
}

void ss::manager::computersManager::Remove(computer computer)
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"1. Iniciando processo de remoção de computador | saIPCControl: " + this->IPCControlToString());

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2. Verificando se este computador é o host | saIPCControl: " + this->IPCControlToString());

    //Se for host, remove o computador informado
    if(this->isHost)
    {
        logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2.1. Remoção de participante como host | saIPCControl: " + this->IPCControlToString());

        logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2.2. Aguardando liberação de semaforo | saIPCControl: " + this->IPCControlToString());

        sem_wait(this->sem);

        logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2.3. Aguardando status READY | saIPCControl: " + this->IPCControlToString());

        while((*(uint8_t*)this->saIPCControl) != READY);

        logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2.4. Definindo status como REMOVE | saIPCControl: " + this->IPCControlToString());

        *(uint8_t*)this->saIPCControl = REMOVE;

        logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2.5. Aguardando status WAIT | saIPCControl: " + this->IPCControlToString());

        while((*(uint8_t*)this->saIPCControl) != WAIT);

        logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2.6. Escrevendo PC na memória compartilhada | saIPCControl: " + this->IPCControlToString());

        WriteOnSA(computer);

        logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2.7. Definindo status como END | saIPCControl: " + this->IPCControlToString());

        *(uint8_t*)this->saIPCControl = END;

        logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2.8. Liberando semaforo | saIPCControl: " + this->IPCControlToString());

        sem_post(this->sem);
    }
    //Se não for host, envia a solicitação para o host para remover o computador informado
    else
    {
        logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2.1 Remoção de participante como participante | saIPCControl: " + this->IPCControlToString());

        logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2.2 Enviando mensagem de saída para o host | saIPCControl: " + this->IPCControlToString());

        this->SendExitMessage(computer);
    }

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"3. Finalizando processo de remoção de computador | saIPCControl: " + this->IPCControlToString());
}

void ss::manager::computersManager::RemoveResponse()
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"1. Iniciando processo de resposta de remoção de computador | saIPCControl: " + this->IPCControlToString());

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2. Definindo status como WAIT | saIPCControl: " + this->IPCControlToString());

    *(uint8_t*)this->saIPCControl = WAIT;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"3. Aguardando status END | saIPCControl: " + this->IPCControlToString());

    while((*(uint8_t*)this->saIPCControl) != END);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"4. Lendo PC da memória compartilhada | saIPCControl: " + this->IPCControlToString());

    computer pcd = ReadFromSA();

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"5. Procurando participante para remoção | saIPCControl: " + this->IPCControlToString());

    auto index = manager::computersManager::IndexOf(pcd.GetIPV4());

    if(index == manager::computersManager::npos)
    {
        logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"6. Participante não encontrado para remoção | saIPCControl: " + this->IPCControlToString());
        logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"7. Finalizando processo de resposta de remoção de computador | saIPCControl: " + this->IPCControlToString());

        return;    
    }

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"6. Removendo participante do sistema | saIPCControl: " + this->IPCControlToString());

    this->_data.erase(this->_data.begin() + index);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"7. Atualizando contador de modificações da lista de computadores | saIPCControl: " + this->IPCControlToString());

    this->UpdateLastUpdate();

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"8. Finalizando processo de resposta de remoção de computador | saIPCControl: " + this->IPCControlToString());
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
    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"1. Iniciando processo de resposta de atualização de computador | saIPCControl: " + this->IPCControlToString());

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2. Definindo status como WAIT | saIPCControl: " + this->IPCControlToString());

    *(uint8_t*)this->saIPCControl = WAIT;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"3. Aguardando status END | saIPCControl: " + this->IPCControlToString());

    while((*(uint8_t*)this->saIPCControl) != END);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"4. Lendo PC da memória compartilhada | saIPCControl: " + this->IPCControlToString());

    computer pcd = ReadFromSA();

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"5. Procurando participante para atualização | saIPCControl: " + this->IPCControlToString());

    auto index = manager::computersManager::IndexOf(pcd.GetIPV4());

    if(index == manager::computersManager::npos)
    {
        // TODO: Necessário implementar trativa, pois se receber uma atualização de um computador que não existe, significa alguma inconsistencia
        logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"6. Participante não encontrado para atualização | saIPCControl: " + this->IPCControlToString());
        return;    
    }

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"6. Atualizando participante no sistema | saIPCControl: " + this->IPCControlToString());

    this->_data.at(index) = pcd;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"7. Atualizando contador de modificações da lista de computadores | saIPCControl: " + this->IPCControlToString());

    this->UpdateLastUpdate();

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"8. Finalizando processo de resposta de atualização de computador | saIPCControl: " + this->IPCControlToString());
}

void manager::computersManager::Insert(computer computer)
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"1. Iniciando processo de inserção de computador | saIPCControl: " + this->IPCControlToString());

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2. Aguardando liberação de semaforo | saIPCControl: " + this->IPCControlToString());

    sem_wait(this->sem);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"3. Aguardando status READY | saIPCControl: " + this->IPCControlToString());

    while((*(uint8_t*)this->saIPCControl) != READY);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"4. Definindo status como INSERT | saIPCControl: " + this->IPCControlToString());

    *(uint8_t*)this->saIPCControl = INSERT;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"5. Aguardando status WAIT | saIPCControl: " + this->IPCControlToString());

    while((*(uint8_t*)this->saIPCControl) != WAIT);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"6. Escrevendo PC na memória compartilhada | saIPCControl: " + this->IPCControlToString());

    WriteOnSA(computer);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"7. Definindo status como END | saIPCControl: " + this->IPCControlToString());

    *(uint8_t*)this->saIPCControl = END;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"8. Liberando semaforo | saIPCControl: " + this->IPCControlToString());

    sem_post(this->sem);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"9. Finalizando processo de inserção de computador | saIPCControl: " + this->IPCControlToString());
}

void manager::computersManager::InsertResponse()
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"1. Iniciando processo de resposta de inserção de computador | saIPCControl: " + this->IPCControlToString());

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2. Definindo status como WAIT | saIPCControl: " + this->IPCControlToString());

    *(uint8_t*)this->saIPCControl = WAIT;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"3. Aguardando status END | saIPCControl: " + this->IPCControlToString());

    while((*(uint8_t*)this->saIPCControl) != END);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"4. Lendo PC da memória compartilhada | saIPCControl: " + this->IPCControlToString());

    computer pcd = ReadFromSA();

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"5. Verificando se participante já não está presente | saIPCControl: " + this->IPCControlToString());

    if(manager::computersManager::IndexOf(pcd.GetIPV4()) != manager::computersManager::npos)
    {
        //TODO: Implementar retorno para indicar que já existe na lista
        logger::GetInstance().Debug(__PRETTY_FUNCTION__, "6. Computador já registrado no sistema | saIPCControl: " + this->IPCControlToString());
        logger::GetInstance().Debug(__PRETTY_FUNCTION__, "7. Finalizando processo de resposta de inserção de computador | saIPCControl: " + this->IPCControlToString());
        return;
    }

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"6. Atribuir id ao novo participante | saIPCControl: " + this->IPCControlToString());
    
    pcd.SetID(this->GetNewID());

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"7. Adicionando participante ao sistema | saIPCControl: " + this->IPCControlToString());
    
    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"8. Id atribuído: " + std::to_string(pcd.GetID()) + " | saIPCControl: " + this->IPCControlToString());

    this->_data.push_back(pcd);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"9. Atualizando contador de modificações da lista de computadores | saIPCControl: " + this->IPCControlToString());

    this->UpdateLastUpdate();

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"10. Finalizando processo de resposta de inserção de computador | saIPCControl: " + this->IPCControlToString());
}

void manager::computersManager::HandleRequest()
{
    // logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Pronto para nova requisição");

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

    case RMHOST:

        logger::GetInstance().Debug(__PRETTY_FUNCTION__, "Removendo computador host");
        this->ClearHostResponse();
        break;

    default:
        break;
    }

    *(uint8_t*)this->saIPCControl = READY;

    thread::Sleep(500);
}

computer manager::computersManager::ReadFromSA() const
{
    return computer(*(computer::computerData*)this->saComputerData);

    // return computer {
    //         std::string((char*)this->saHostname),
    //         network::MAC((byte*)this->saMAC),
    //         network::IPV4(*(int32_t*)this->saIPV4),
    //         static_cast<computer::computerStatus>(*(uint8_t*)this->saStatus)
    //     };
}

void manager::computersManager::WriteOnSA(computer pcd)
{
    //Completa o vetor do hostname com zeros
    // std::fill((char*)this->saHostname, (char*)this->saHostname + 64, 0);

    //Copia o hostname para o vetor
    // std::memcpy((char*)this->saHostname, pcd.GetName().c_str(), pcd.GetName().size());
    
    //Copia o endereço MAC para o vetor
    // std::memcpy((char*)this->saMAC, pcd.GetMAC().Get(), 6);
    
    //Copia o IPV4 para a variavel
    // *(int32_t*)this->saIPV4 = pcd.GetIPV4().Get();

    //Copia o Status para a variável
    // *(uint8_t*)this->saStatus = static_cast<uint8_t>(pcd.GetStatus());

    //escreve os dados do computador na memória compartilhada
    *(computer::computerData*)this->saComputerData = pcd.ToComputerData();
}

void manager::computersManager::__Insert(computer computer)
{
    this->_data.push_back(computer);
}

computer ss::manager::computersManager::GetHost()
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"Iniciando processo de obtenção do computador host");

    if(!this->IsHostSeted())
    {
        // Presumisse que o host deve estar definido para utilização do GetHost
        throw std::runtime_error("Host não definido");
    }

    if(hostComputer == nullptr)
    {
        // Entrar em contato com o processo manager para obter o host

        sem_wait(this->sem);

        while((*(uint8_t*)this->saIPCControl) != READY);

        *(uint8_t*)this->saIPCControl = GETHOST;

        while((*(uint8_t*)this->saIPCControl) != WAIT);

        this->hostComputer = new computer(this->ReadFromSA());

        *(uint8_t*)this->saIPCControl = END;

        sem_post(this->sem);

        logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"Host definido como: " + this->hostComputer->GetName() + "|" + this->hostComputer->GetIPV4().ToString());
    }

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"Finalizando processo de obtenção do computador host");

    return *this->hostComputer;
}

void ss::manager::computersManager::GetHostResponse()
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"Iniciando processo de resposta de obtenção do computador host");

    this->WriteOnSA(*this->hostComputer);

    *(uint8_t*)this->saIPCControl = WAIT;

    while((*(uint8_t*)this->saIPCControl) != END);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"Finalizando processo de resposta de obtenção do computador host");
}

void ss::manager::computersManager::SetHost(computer computer)
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"1 Aguardando liberação semaforo | saIPCControl: " + this->IPCControlToString());

    sem_wait(this->sem);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2 Aguardando status READY | saIPCControl: " + this->IPCControlToString());
    
    while((*(uint8_t*)this->saIPCControl) != READY);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"3 Definindo status como SETHOST | saIPCControl: " + this->IPCControlToString());

    *(uint8_t*)this->saIPCControl = SETHOST;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"4 Aguardando status WAIT | saIPCControl: " + this->IPCControlToString());

    while((*(uint8_t*)this->saIPCControl) != WAIT);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"5 Escrevendo PC na memória compartilhada | saIPCControl: " + this->IPCControlToString());

    WriteOnSA(computer);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"6 Definindo status como END | saIPCControl: " + this->IPCControlToString());

    *(uint8_t*)this->saIPCControl = END;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"7 Liberando semaforo | saIPCControl: " + this->IPCControlToString());

    sem_post(this->sem);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"8 Fim | saIPCControl: " + this->IPCControlToString());
}

void ss::manager::computersManager::SetHostResponse()
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"1 Definindo status como WAIT | saIPCControl: " + this->IPCControlToString());

    *(uint8_t*)this->saIPCControl = WAIT;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2 Aguardando status END | saIPCControl: " + this->IPCControlToString());

    while((*(uint8_t*)this->saIPCControl) != END);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"3 Lendo PC da memória compartilhada | saIPCControl: " + this->IPCControlToString());

    computer pcd = ReadFromSA();

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"4 Definindo PC host | saIPCControl: " + this->IPCControlToString());

    this->hostComputer = new computer(pcd);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"5 Definindo variavel de controle de definição do host como true | saIPCControl: " + this->IPCControlToString());

    *(bool*)this->saIsHostSeted = true;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"6 Atualizando contagem de atualização do Manager | saIPCControl: " + this->IPCControlToString());

    this->UpdateLastUpdate();

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"7 Fim | saIPCControl: " + this->IPCControlToString());
}

void ss::manager::computersManager::ClearHost()
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"1 Aguardando liberação semaforo | saIPCControl: " + this->IPCControlToString());

    sem_wait(this->sem);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2 Aguardando status READY | saIPCControl: " + this->IPCControlToString());

    while((*(uint8_t*)this->saIPCControl) != READY);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"3 Definindo status como RMHOST | saIPCControl: " + this->IPCControlToString());

    *(uint8_t*)this->saIPCControl = RMHOST;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"4 Aguardando status WAIT | saIPCControl: " + this->IPCControlToString());

    while((*(uint8_t*)this->saIPCControl) != WAIT);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"5 Definindo status como END | saIPCControl: " + this->IPCControlToString());

    *(uint8_t*)this->saIPCControl = END;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"6 Liberando semaforo | saIPCControl: " + this->IPCControlToString());

    sem_post(this->sem);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"7 Fim | saIPCControl: " + this->IPCControlToString());
}

void ss::manager::computersManager::ClearHostResponse()
{
    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"1 Definindo status como WAIT | saIPCControl: " + this->IPCControlToString());

    *(uint8_t*)this->saIPCControl = WAIT;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"2 Aguardando status END | saIPCControl: " + this->IPCControlToString());

    while((*(uint8_t*)this->saIPCControl) != END);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"3 Definindo PC host como nullptr | saIPCControl: " + this->IPCControlToString());

    delete this->hostComputer;
    this->hostComputer = nullptr;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"4 Definindo variavel de controle de definição do host como false | saIPCControl: " + this->IPCControlToString());

    *(bool*)this->saIsHostSeted = false;

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"5 Atualizando contagem de atualização do Manager | saIPCControl: " + this->IPCControlToString());

    this->UpdateLastUpdate();

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"6 Fim | saIPCControl: " + this->IPCControlToString());
}

bool ss::manager::computersManager::IsHostSeted() const
{
    return *(bool*)this->saIsHostSeted;
}

std::string ss::manager::computersManager::IPCControlToString() const
{
    return IPCControlToString(*(uint8_t*)this->saIPCControl);
}

std::string ss::manager::computersManager::IPCControlToString(int control)
{
    switch (control)
    {
    case INSERT:
        return "INSERT";
        break;
    case UPDATE:
        return "UPDATE";
        break;
    case GET:
        return "GET";
        break;
    case READY:
        return "READY";
        break;
    case WAIT:
        return "WAIT";
        break;
    case NEXT:
        return "NEXT";
        break;
    case END:
        return "END";
        break;
    case ENDLIST:
        return "ENDLIST";
        break;
    case SIZE:
        return "SIZE";
        break;
    case REMOVE:
        return "REMOVE";
        break;
    case GETHOST:
        return "GETHOST";
        break;
    case SETHOST:
        return "SETHOST";
        break;
    case ISSETED:
        return "ISSETED";
        break;
    case YES:
        return "YES";
        break;
    case NO:
        return "NO";
        break;
    case RMHOST:
        return "RMHOST";
        break;
    default:
        return "UNKNOWN";
        break;
    }
}

int ss::manager::computersManager::GetNewID()
{
    auto biggerId = 0;

    if(this->_data.size() != 0)
    {
        auto biggerId = 0;

        for(auto &pcd : this->_data)
        {
            if(pcd.GetID() > biggerId)
                biggerId = pcd.GetID();
        }
    }

    return biggerId + 1;
}

void ss::manager::computersManager::SendPCListUpdate()
{
    while(this->pcListUpdateThread.joinable())
    {
        logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"Aguardando finalização da thread de atualização de lista de computadores");
        // thread::Sleep(1000);
        this->pcListUpdateThread.join();
    }

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"Iniciando thread de atualização de lista de computadores");

    this->pcListUpdateThread = std::thread([this]() { this->SendPCListUpdateOverNetwork(); });
}

void ss::manager::computersManager::SendPCListUpdateOverNetwork()
{
    sem_wait(this->sem);

    auto socket = network::Socket(IPPROTO_UDP);
    socket.SetConfig(SO_BROADCAST, 1);      //Habilita broadcast
    int sequence = 0;

    auto packet = network::packet(this->thisComputer, network::packet::LISTUPDATE, computersManager::MANAGER_LEADER_PORT, sequence++);

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"1. Enviando mensagem inicial LISTUPDATE");

    socket.Send(packet, computersManager::PCLIST_UPDATE_PORT, INADDR_BROADCAST);

    int i = 0;
    for(; i < this->_data.size(); i++)
    {
        thread::Sleep(100);
        
        logger::GetInstance().Debug(__PRETTY_FUNCTION__ , std::to_string(i + 2) +  ". Enviando dados do computador: " + this->_data.at(i).GetName() + "|" + this->_data.at(i).GetIPV4().ToString());

        packet = network::packet(this->thisComputer, network::packet::PCDATA, computersManager::MANAGER_LEADER_PORT, sequence++, this->_data.at(i).ToComputerData());

        socket.Send(packet, computersManager::PCLIST_UPDATE_PORT, INADDR_BROADCAST);
    }

    logger::GetInstance().Debug(__PRETTY_FUNCTION__ , std::to_string(i + 2) +  ". Enviando mensagem final ENDLIST");

    packet = network::packet(this->thisComputer, network::packet::ENDLIST, computersManager::MANAGER_LEADER_PORT, sequence++);

    socket.Send(packet, computersManager::PCLIST_UPDATE_PORT, INADDR_BROADCAST);

    sem_post(this->sem);
}

void ss::manager::computersManager::ListenPCListUpdate()
{
    auto socket = network::Socket(IPPROTO_UDP);
    timeval timeout = {.tv_sec = 5 };
    socket.SetConfig(SO_RCVTIMEO, timeout);
    auto port = computersManager::PCLIST_UPDATE_PORT;
    socket.Bind(port);

    while(!this->isHost)
    {
        auto packet = socket.receivePacket();

        if(packet.IsDataInicialized() && packet.GetPacket().message == network::packet::LISTUPDATE)
        {
            logger::GetInstance().Debug(__PRETTY_FUNCTION__ ,"1. Recebido mensagem de atualização de lista de computadores");

            computers newPCList;

            bool keepReceiving = true;

            while(keepReceiving)
            {
                packet = socket.receivePacket();

                if(!packet.IsDataInicialized())
                {
                    logger::GetInstance().Error(__PRETTY_FUNCTION__ , std::to_string(newPCList.size() + 2) + ". Dados não foram recebidos completamente. ");
                    keepReceiving = false;
                    continue;
                }
                else
                {
                    switch (packet.GetPacket().message)
                    {
                    case network::packet::PCDATA:
                        newPCList.push_back(computer(packet.GetPacket().payload));
                        logger::GetInstance().Debug(__PRETTY_FUNCTION__ , std::to_string(newPCList.size() + 2) + ". Dados do computador recebidos: " + newPCList.back().GetName() + "|" + newPCList.back().GetIPV4().ToString());
                        break;
                    case network::packet::ENDLIST:
                        logger::GetInstance().Debug(__PRETTY_FUNCTION__ , std::to_string(newPCList.size() + 2) + ". Fim da lista de computadores");
                        keepReceiving = false;
                        break;
                    default:
                        logger::GetInstance().Error(__PRETTY_FUNCTION__ , std::to_string(newPCList.size() + 2) + ". Mensagem inesperada recebido: " + std::to_string(packet.GetPacket().message));
                        break;
                    }
                }
            }

            logger::GetInstance().Debug(__PRETTY_FUNCTION__, std::to_string(newPCList.size() + 2) + ". Aguardando liberação de semaforo");

            sem_wait(this->sem);

            logger::GetInstance().Debug(__PRETTY_FUNCTION__, std::to_string(newPCList.size() + 2) + ". Definindo nova lista de computadores");

            this->_data = newPCList;

            logger::GetInstance().Debug(__PRETTY_FUNCTION__, std::to_string(newPCList.size() + 2) + ". Incrementando contador de atualização");

            this->UpdateLastUpdate();

            logger::GetInstance().Debug(__PRETTY_FUNCTION__, std::to_string(newPCList.size() + 2) + ". Liberando semaforo");

            sem_post(this->sem);
        }
    }
}
