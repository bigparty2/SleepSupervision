
#include "manager.hpp"

using namespace ss;

manager::computersManager::computersManager()
{
    //inicialização dos ponteiros para memória compartilhada
    this->saLastUpdate = mmap(NULL, sizeof(uint64_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    this->saHostname = mmap(NULL, sizeof(char)*64, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    this->saMAC = mmap(NULL, sizeof(char)*6, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    this->saIPV4 = mmap(NULL, sizeof(int32_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    this->saStatus = mmap(NULL, sizeof(uint8_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    this->saIPCControl = mmap(NULL, sizeof(uint8_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    //ultima atualização
    *(uint64_t*)this->saLastUpdate = 0;

    //controle comunicação entre processos
    *(uint64_t*)this->saIPCControl = WAIT;

    //sem_close(sem);
    sem_unlink(SEM_NAME);

    //inicialização do semaforo (semaforo nomeado) (apenas 1 por vez)
    this->sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);

    //Verificação de erro na abertura do semaforo
    if(this->sem == SEM_FAILED)
    {
        // TODO: Incluir erro na classe LOG
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
}

manager::computersManager::~computersManager()
{
    sem_close(sem);
    sem_unlink(SEM_NAME);
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
        if(this->_data.at(index).name == hostname)
            return index;
    }

    return manager::computersManager::npos;
}

uint64_t manager::computersManager::IndexOf(network::IPV4 ipv4)
{
    for (int index = 0; index < this->_data.size(); index++)
    {
        if(this->_data.at(index).ipv4 == ipv4)
            return index;
    }

    return manager::computersManager::npos;
}

uint64_t manager::computersManager::IndexOf(network::MAC macAddr)
{
    for (int index = 0; index < this->_data.size(); index++)
    {
        if(this->_data.at(index).macAddr == macAddr)
            return index;
    }

    return manager::computersManager::npos;
}

bool manager::computersManager::Remove(network::MAC macAddr)
{
    auto index = manager::computersManager::IndexOf(macAddr);

    if(index == manager::computersManager::npos)
        return false;

    manager::computersManager::RemoveByIndex(index);

    manager::computersManager::UpdateLastUpdate();

    return true;
}

bool manager::computersManager::Remove(network::IPV4 ipv4)
{
    auto index = manager::computersManager::IndexOf(ipv4);

    if(index == manager::computersManager::npos)
        return false;

    manager::computersManager::RemoveByIndex(index);

    manager::computersManager::UpdateLastUpdate();

    return true;
}

bool manager::computersManager::Remove(std::string hostname)
{
    auto index = manager::computersManager::IndexOf(hostname);

    if(index == manager::computersManager::npos)
        return false;

    manager::computersManager::RemoveByIndex(index);

    manager::computersManager::UpdateLastUpdate();

    return true;
}

void manager::computersManager::RemoveByIndex(uint64_t index)
{
    this->_data.erase(this->_data.begin() + index);
}

void manager::computersManager::Update(computer computer)
{
    sem_wait(this->sem);

    while((*(uint8_t*)this->saIPCControl) != READY);

    *(uint8_t*)this->saIPCControl = UPDATE;

    while((*(uint8_t*)this->saIPCControl) != WAIT);

    WriteOnSA(computer);

    *(uint8_t*)this->saIPCControl = END;

    sem_post(this->sem);

    //printf("Atualizou!!");
}

void manager::computersManager::UpdateResponse()
{
    *(uint8_t*)this->saIPCControl = WAIT;

    while((*(uint8_t*)this->saIPCControl) != END);

    computer pcd = ReadFromSA();

    auto index = manager::computersManager::IndexOf(pcd.ipv4);

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

    if(manager::computersManager::IndexOf(pcd.ipv4) != manager::computersManager::npos)
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

        this->InsertResponse();
        break;

    case UPDATE:

        this->UpdateResponse();
        break;
        
    case GET:

        this->GetResponse();
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
    std::memcpy((char*)this->saHostname, pcd.name.c_str(), pcd.name.size());
    
    //Copia o endereço MAC para o vetor
    std::memcpy((char*)this->saMAC, pcd.macAddr.Get(), 6);
    
    //Copia o IPV4 para a variavel
    *(int32_t*)this->saIPV4 = pcd.ipv4.Get();

    //Copia o Status para a variável
    *(uint8_t*)this->saStatus = static_cast<uint8_t>(pcd.status);
}

void manager::computersManager::__Insert(computer computer)
{
    this->_data.push_back(computer);
}