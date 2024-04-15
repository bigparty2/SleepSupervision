#include "interface.hpp"

using namespace ss; 

interface::interfaceManager::terminalSizeManager::terminalSizeManager()
{
    this->UpdateCurrentRes();

    this->resAnterior = this->resAtual;
}

bool interface::interfaceManager::terminalSizeManager::HasChange()
{
    this->UpdateCurrentRes();

    return this->resAtual.x != this->resAnterior.x or this->resAtual.y != this->resAnterior.y;
}

coord interface::interfaceManager::terminalSizeManager::Get()
{
    return this->resAtual;
}

void interface::interfaceManager::terminalSizeManager::UpdateCurrentRes()
{
    winsize currentTerminalRes;

    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &currentTerminalRes) == -1)
    {
        // TODO: Incluir erro ao LOG
        throw std::runtime_error("Falha ao capturar resolução do terminal. Erro: " + std::string(std::strerror(errno)));
    }

    this->resAnterior = this->resAtual;
    this->resAtual = {currentTerminalRes.ws_col, currentTerminalRes.ws_row};

    return;
}

interface::interfaceManager::interfaceManager(ss::manager::computersManager &cm, bool manager)
{
    this->Init(cm, manager);
}

interface::interfaceManager::~interfaceManager()
{
    this->tend.join();

    if(this->isRunning == true)
        this->End();        
}

void interface::interfaceManager::Init(ss::manager::computersManager &cm, bool manager)
{
    //Validação se o sistema já foi inicializado
    if(this->isRunning == true)
    {
        // TODO: Adicionar a classe LOG        
        std::runtime_error("A interface já foi inicializada");    
    }

    //Defini variavel de controle de inicializado/finalizado
    this->isRunning = true;

    //Definição do ponteiro para a estrutura com dados dos computadores no sistema
    this->machinesManager = &cm;

    //Atualiza a data da ultima atualização da tela
    this->lastChange = this->machinesManager->LastUpdate();

    //Atualiza a lista de computadores (manager)
    if(manager) this->machines = this->machinesManager->Get();

    //Inicialização do gerenciador de resolução
    this->res = new interface::interfaceManager::terminalSizeManager();

    //Define se o computador é manager ou não
    this->IsManager = manager;

    //Define método de ordenamento inicial
    this->currentOrder = orderBy::hostnameAsc;
    this->previousOrder = orderBy::hostnameDesc;

    // Pega os dados do computador local
    localComputer.GetComputerInfo();

    //Define para o padrão de caracteres da linguagem corrente
    std::setlocale(LC_ALL, "");

    //salvar as configurações atuais do terminal
    tcgetattr(STDIN_FILENO, &this->oldTermAttr);

    //Define as novas configurações do terminal
    termios newTermAttr = this->oldTermAttr;
    newTermAttr.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newTermAttr);

    //Salva estado previo do console
    std::system("tput smcup #save previous state");

    //Inicializa a thread de output
    this->tout = std::thread(&interface::interfaceManager::OutputManager, this);
    
    //Inicializa a thread de input
    this->tin = std::thread(&interface::interfaceManager::InputManager,this);    
}

void interface::interfaceManager::End()
{
    //Comando para encerramento das threads
    this->threadKeepAlive = false;

    //Desalocando espaço do gerenciador de resolução
    delete this->res;
    this->res = nullptr;

    //Desassociar o ponteiro da lista
    this->machinesManager = nullptr;
    
    //Aguarda encerramento da thread de controle de entrada
    this->tout.join();

    //Restaurando as configurações do terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &this->oldTermAttr);

    //Restaura estado conteúdo anterior do terminal
    std::system("tput rmcup #restore previous state");

    //Aguarda encerramento da thread de controle de saída
    this->tin.join();

    //Define o serviço como parado
    this->isRunning = false;
}

void interface::interfaceManager::InputManager()
{
    while(this->threadKeepAlive)
    {
        //Se tiver algo para ler
        if(KbHit())
        {
            //Ler do teclado
            auto key = std::getchar();

            //Verifica se a tecla precionada é utilizada pelo sistema
            if(!AcceptedKeys(key))
                continue;

            switch (key)
            {
            //Sair do programa
            case (int)'Q':
            case (int)'q':
                this->machinesManager->Remove(localComputer.macAddr);
                this->tend = std::thread(&interface::interfaceManager::End, this);
                break;

            //Controle de setas
            case 27:
            {
                //verifica se possui elemento na tabela para navegar
                if(this->machines.size())
                {             
                    auto command0 = std::getchar();
                    auto command1 = std::getchar();

                    //Esquerda
                    if(command0 == 91 and command1 == 68)
                    {
                        //seleção no cabeçaho
                        if(selection < -1)
                        {
                            this->selection++;
                            this->CriticalRegion();
                        }

                        //seleção na tabela
                        else
                        {
                            if(CurrentPage > 1)
                            {
                                CurrentPage--;
                                selection = 0;
                                this->CriticalRegion();
                            }
                        }
                    }
                        
                    //Direita
                    else if(command0 == 91 and command1 == 67)
                    {
                        //seleção no cabelho
                        if(selection < 0 and selection > -4)
                        {
                            selection--;
                            this->CriticalRegion();
                        }
                        //Tabela
                        else
                        {
                            if(CurrentPage < numOfPages)
                            {
                                CurrentPage++;
                                selection = 0;
                                this->CriticalRegion();
                            }
                        }
                    }

                    //Baixo
                    else if(command0 == 91 and command1 == 66)
                    {
                        //Cabeçalho
                        if(selection < 0)
                        {
                            selection = 0;

                            this->CriticalRegion();
                        }
                        //Tabela
                        else
                        {
                            if(selection < this->currentRows - 1)
                            {
                                selection++;

                                this->CriticalRegion();
                            }
                        }
                    }

                    //Cima
                    else if(command0 == 91 and command1 == 65)
                    {
                        //Tabela
                        if(selection > -1)
                        {
                            selection--;

                            this->CriticalRegion();
                        }
                    }
                }

                break;
            }
                           
            //Espaço
            case 32:

                //verifica se possui elemento na tabela para iteragir
                if(this->machines.size())
                {
                    //cabeçalho
                    if(selection < 0)
                    {
                        switch (selection)
                        {
                        //hostname
                        case -1:
                            currentOrder = (currentOrder == orderBy::hostnameAsc ? orderBy::hostnameDesc : orderBy::hostnameAsc);
                            break;

                        //mac
                        case -2:
                            currentOrder = (currentOrder == orderBy::MACAsc ? orderBy::MACDesc :orderBy::MACAsc);
                            break;
                        
                        //ip
                        case -3:
                            currentOrder = (currentOrder == orderBy::IPAsc ? orderBy::IPDesc : orderBy::IPAsc);
                            break;

                        //status
                        case -4:
                            currentOrder = (currentOrder == orderBy::StatusAsc ? orderBy::StatusDesc : orderBy::StatusAsc);
                            break;
                        
                        default:
                            throw std::runtime_error("Seleção de elemento com estado indefinido.");
                        }

                        this->CriticalRegion();
                    }

                    //Tabela
                    else
                    {
                        network::wakeOnLan::Awake(this->machines.at(selection + ((CurrentPage - 1) * rowsPerPage)).macAddr.ToString());
                    }

                }    

                break;
                
            default:
                break;
            }
        }
   
    }
}

bool interface::interfaceManager::KbHit()
{
    struct pollfd fds;
    int ret;
    fds.fd = 0;
    fds.events = POLLIN;
    ret = poll(&fds, 1, 0);
    if(ret == 1)
        return true;
    else if(ret == 0)
        return false;
    else
    {
        //TODO: Incluir erro na classe LOG
        throw std::runtime_error("Falha na captura do STDIN");    
    }
}

bool interface::interfaceManager::AcceptedKeys(int key)
{
    for(const auto &akey : this->keysList)
        if(akey == key)
            return true;

    return false;
}

void interface::interfaceManager::OutputManager()
{
    //Ocultar cursor
    interfaceManager::HideCursor();

    //Bloqueia rolagem de tela
    interfaceManager::LockScrolling();

    //Ajuste metodo de ordenamento
    this->AdjustOrdering();

    //Desenha em tela
    this->Draw();

    while(this->threadKeepAlive)
    {
        auto currentChange = this->machinesManager->LastUpdate();

        if(this->res->HasChange() or (currentChange != this->lastChange) or this->inputRedraw)
        {
            //Verifica se houve mudança na lista de computadores
            if(currentChange != this->lastChange and this->IsManager)
            {
                this->machines = this->machinesManager->Get();
                this->lastChange = this->machinesManager->LastUpdate();
            }
            
            //Reseta controle de renderização
            this->inputRedraw = false;

            //Ajuste metodo de ordenamento
            this->AdjustOrdering();

            //Desenha em tela
            this->Draw();
        }

        thread::Sleep(THREAD_SLEEP_TIME);
    }

    //Restaurar cursor
    interfaceManager::ShowCursor();

    //Restaura a rolagem de tela
    interfaceManager::UnlockScrolling();

    //Limpa a tela
    interfaceManager::Clear();
}

void interface::interfaceManager::Draw()
{
    //Captura dados da resolução atual do terminal
    auto colunasX = this->res->Get().x;
    auto linhasY = this->res->Get().y;

    //Atualiza informação de posicionamento de elementos
    this->frameBottomLinePos = linhasY - this->linesAfterEndFrame;
    this->headerLinePos = frameTopLinePos + 1;
    this->firstRowPos = headerLinePos + 1;
    this->commandInfoLinePos = frameBottomLinePos + 3;
    this->headerElementLenght = floor(colunasX - (frameMargin*2) - 5) / 4;
    this->headerElement0Pos = frameMargin + 2;
    this->headerElement1Pos = headerElement0Pos + 1;
    this->headerElement2Pos = headerElement1Pos + 1;
    this->headerElement3Pos = headerElement2Pos + 1;
    this->rowsPerPage = frameBottomLinePos - headerLinePos - 1;
    this->numOfPages = 0;

    //Limpa tela
    this->Clear();

    //Print do titulo
    this->SetTextBlackBackgroundWrite();
    std::cout << string::ToCenter(this->title, colunasX);
    this->SetColorDefault();

    //Verifica se é participante
    if(!this->IsManager)
    {
        if(this->machinesManager->IsHostSeted())
        {
            auto host = this->machinesManager->GetHost();
            this->GotoYX((this->frameBottomLinePos+this->frameTopLinePos)/2, 1);
            std::cout << string::ToCenter((this->hostMsg + host.GetName() + "|" + host.GetMAC().ToString() + "|" + host.GetIPV4().ToString()), colunasX);

            //Print instruções de comando
            this->GotoYX(this->commandInfoLinePos, 1);
            std::cout << string::ToCenter(this->noDataCommand, colunasX);
        }
        else
        {
            this->GotoYX((this->frameBottomLinePos+this->frameTopLinePos)/2, 1);
            std::cout << string::ToCenter(this->clientWaittMsg, colunasX);

            //Print instruções de comando
            this->GotoYX(this->commandInfoLinePos, 1);
            std::cout << string::ToCenter(this->noDataCommand, colunasX);
        }
    }
    //Verifica se há computadores a ser exibidos
    else if(this->machines.size())
    {
        //Atualiza informação de posicionamento de elementos
        this->numOfPages = ceil((float)this->machines.size()/rowsPerPage);

        //Print do titulo da tabela
        this->GotoYX(this->frameTopLinePos - 1, 1);
        std::cout << string::ToCenter(this->tableTitle, colunasX);

        //Print do frame
        this->PrintFrame();

        //Cabeçalho
        this->PrintHeader();

        //Tabela
        this->PrintTable();

        //Print informações das paginas
        this->PrintPagesInfo();

        //Print instruções de comando
        this->GotoYX(this->commandInfoLinePos, 1);
        std::cout << string::ToCenter((this->selection < 0 ? this->headerCommand : this->tableCommand), colunasX);
    }
    else
    {
        //Exibe mensagem de ausencia de computadores
        this->GotoYX((this->frameBottomLinePos+this->frameTopLinePos)/2, 1);
        std::cout << string::ToCenter(this->noDataMsg, colunasX);

        //Print instruções de comando
        this->GotoYX(this->commandInfoLinePos, 1);
        std::cout << string::ToCenter(this->noDataCommand, colunasX);
    }

    //Print rodapé
    this->PrintFooter();

    //Libera buffer de saída
    std::fflush(stdout);
}

void interface::interfaceManager::PrintFrame()
{
    //Print do frame (linha superior)
    this->GotoYX(frameTopLinePos, this->frameMargin + 1);
    std::cout << std::string(this->frameTopLeft + string::Repeat(res->Get().x - (this->frameMargin + 1) * 2, this->frameHorizontal) + this->frameTopRight);

    //Print do frame (colunas)
    for(int i = this->frameTopLinePos + 1; i < frameBottomLinePos; i++)
    {
        this->GotoYX(i,this->frameMargin + 1);
        std::cout << this->frameVetical;

        this->GotoYX(i, res->Get().x - this->frameMargin);
        std::cout << this->frameVetical;
    }

    //Print do frame (linha inferior)
    this->GotoYX(this->frameBottomLinePos, this->frameMargin + 1);
    std::cout << std::string(this->frameBottomLeft + string::Repeat(res->Get().x - (this->frameMargin + 1) * 2, this->frameHorizontal) + this->frameBottomRight);
}

void interface::interfaceManager::PrintFooter()
{
    this->GotoYX(res->Get().y, 1);
    this->SetTextBlackBackgroundWrite();
    std::cout << string::ToCenter("Hostname: " + localComputer.name, res->Get().x);
    this->GotoYX(res->Get().y, 1);
    std::cout << "Modo: " << (this->IsManager ? "Gerenciador  │" : "Participante │");
    this->GotoYX(res->Get().y, res->Get().x - 21);
    std::cout << "│  Status: Running   ";
    this->SetColorDefault();
}

void interface::interfaceManager::PrintHeader()
{
    //hostname
    this->GotoYX(this->headerLinePos, this->headerElement0Pos);
    
    if(selection == -1)
        interface::interfaceManager::SetTextBlackBackgroundWrite();
    std::cout << string::ToCenter(this->headerElement0, (currentOrder == orderBy::hostnameAsc or currentOrder == orderBy::hostnameDesc ? this->headerElementLenght + 2 : this->headerElementLenght));
    if(selection == -1)
        interface::interfaceManager::SetColorDefault();

    std::cout << "│";

    //mac
    if(this->selection == -2)
        this->SetTextBlackBackgroundWrite();
    std::cout << string::ToCenter(this->headerElement1, (currentOrder == orderBy::MACAsc or currentOrder == orderBy::MACDesc ? this->headerElementLenght + 2 : this->headerElementLenght));
    if(this->selection == -2)
        this->SetColorDefault();

    std::cout << "│";

    //ip
    if(this->selection == -3)
        this->SetTextBlackBackgroundWrite();
    std::cout << string::ToCenter(this->headerElement2, (currentOrder == orderBy::IPAsc or currentOrder == orderBy::IPDesc ? this->headerElementLenght + 2 : this->headerElementLenght));
    if(this->selection == -3)
        this->SetColorDefault();

    std::cout << "│";

    //status
    if(this->selection == -4)
        this->SetTextBlackBackgroundWrite();
    std::cout << string::ToCenter(this->headerElement3, (currentOrder == orderBy::StatusDesc or currentOrder == orderBy::StatusAsc ? this->headerElementLenght + 2 : this->headerElementLenght));
    if(this->selection == -4)
        this->SetColorDefault();
}

void interface::interfaceManager::PrintTable()
{
    auto pcs = this->PcsOfCurrentPage();
    auto line = this->firstRowPos;

    for(int i = 0; i < pcs.size(); i++, line++)
    {
        //Verifica se o tamanho do host name não ultrapassa o tamanho da coluna
        if(pcs.at(i).name.length() > this->headerElementLenght - 4)
            pcs.at(i).name = pcs.at(i).name.substr(0, pcs.at(i).name.size() - 4) + "...";

        //vai para inicio da linha
        this->GotoYX(line, this->headerElement0Pos);

        //Verifica se a linha selecionada é a atual
        if(selection == i)
            this->SetTextBlackBackgroundWrite();

        //std::cout << string::ToCenter(std::string(pcs.at(i).hostname + std::to_string(pcs.at(i).hostname.size())), this->headerElementLenght);
        std::cout << string::ToCenter(pcs.at(i).name, this->headerElementLenght);
        std::cout << "│";
        std::cout << string::ToCenter(pcs.at(i).macAddr.ToString(), this->headerElementLenght);
        std::cout << "│";
        std::cout << string::ToCenter(pcs.at(i).ipv4.ToString(), this->headerElementLenght);
        std::cout << "│";
        std::cout << string::ToCenter(computer::StatusToStringBR(pcs.at(i).status), this->headerElementLenght);

        if(selection == i)
            this->SetColorDefault();
    }
}

void interface::interfaceManager::PrintPagesInfo()
{
    this->GotoYX(frameBottomLinePos + 1, 1 );

    if(CurrentPage < numOfPages and CurrentPage > 1)
        std::cout << string::ToCenter("<         >", this->res->Get().x);
    else if(CurrentPage == numOfPages and CurrentPage > 1)
        std::cout << string::ToCenter("<          ", this->res->Get().x);
    else if(CurrentPage < numOfPages and CurrentPage == 1)
        std::cout << string::ToCenter("          >", this->res->Get().x);

    this->GotoYX(frameBottomLinePos + 1, this->res->Get().x/2);
    std::cout << this->CurrentPage;
}

computers interface::interfaceManager::PcsOfCurrentPage()
{
    computers toReturn;

    //Copia para retorno
    for(int index = (this->CurrentPage - 1) * this->rowsPerPage; index < ((this->rowsPerPage * this->CurrentPage) >= machines.size() ? machines.size() : this->rowsPerPage * this->CurrentPage) ; index++)
    {
        toReturn.push_back(machines.at(index));
    }

    this->currentRows = toReturn.size();

    return toReturn;
}

ss::coord interface::interfaceManager::GetCursorPosition()
{
    char buf[32];
    int y, x;
    struct termios ttystate, ttysave;
    tcgetattr(STDIN_FILENO, &ttystate);
    ttysave = ttystate;
    ttystate.c_lflag &= ~(ICANON | ECHO);
    ttystate.c_cc[VMIN] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
    sprintf(buf, "\033[6n");
    tcflush(STDIN_FILENO, TCIFLUSH);
    write(1, buf, strlen(buf));
    scanf("\033[%d;%dR", &y, &x);
    tcsetattr(STDIN_FILENO, TCSANOW, &ttysave);
    return {(unsigned short)x, (unsigned short)y};
}

void interface::interfaceManager::Clear()
{
    //clear from top to bottom
    std::system("clear");

    //goto 1,1
    //printf("\033[1;1H");

    //flush
    fflush(stdout);
}

void interface::interfaceManager::GotoYX(int y, int x)
{
    //Goto X,Y
    printf("\033[%d;%dH", y, x);

    //flush
    fflush(stdout);
}

void interface::interfaceManager::HideCursor()
{
    //Ocultar cursor
    printf("\033[?25l");

    //flush
    fflush(stdout);
}

void interface::interfaceManager::ShowCursor()
{
    //Ocultar cursor
    printf("\033[?25h");

    //flush
    fflush(stdout);
}

void interface::interfaceManager::LockScrolling()
{
    //BLoquear a rolagem
    printf("\033[r");

    //flush
    fflush(stdout);
}

void interface::interfaceManager::UnlockScrolling()
{
    //Restaurar a rolagem
    printf("\033[r");

    //flush
    fflush(stdout);
}

void interface::interfaceManager::Join()
{
    while(this->isRunning)
    {
        thread::Sleep(THREAD_SLEEP_TIME);
    }
}

void interface::interfaceManager::SetTextBlackBackgroundWrite()
{
    printf("\033[30;47m");
}

void interface::interfaceManager::SetColorDefault()
{
    printf("\033[0m");
}

void interface::interfaceManager::CriticalRegion()
{
    std::lock_guard<std::mutex> guard(this->iMutex);
    this->inputRedraw = true;
}

void interface::interfaceManager::AdjustOrdering()
{
    //Evita reordenamento desnecessáro
    if(previousOrder == currentOrder)
        return;

    //Corrige string anterior
    switch (previousOrder)
    {
    case orderBy::hostnameAsc:
    case orderBy::hostnameDesc:
        this->headerElement0.pop_back();
        this->headerElement0.pop_back();
        this->headerElement0.pop_back();
        this->headerElement0 += " ";
        break;
    case orderBy::MACAsc:
    case orderBy::MACDesc:
        this->headerElement1.pop_back();
        this->headerElement1.pop_back();
        this->headerElement1.pop_back();
        this->headerElement1 += " ";
        break;
    case orderBy::IPAsc:
    case orderBy::IPDesc:
        this->headerElement2.pop_back();
        this->headerElement2.pop_back();
        this->headerElement2.pop_back();
        this->headerElement2 += " ";
        break;
    case orderBy::StatusAsc:
    case orderBy::StatusDesc:
        this->headerElement3.pop_back();
        this->headerElement3.pop_back();
        this->headerElement3.pop_back();
        this->headerElement3 += " ";
        break;  
    default:
        throw std::runtime_error("Estado do ordenamento não é esperado. (previousOrder)");
    }

    //Correção da string atual e ordenamento
    //TODO: Metodo de ordenamento
    //TODO: enum asc ou desc para definição do metodo de ordenamento
    switch (currentOrder)
    {
    case orderBy::hostnameAsc:
        this->headerElement0.pop_back();
        this->headerElement0 += "▲";
        //order hostname asc
        break;
    
    case orderBy::hostnameDesc:
        this->headerElement0.pop_back();
        this->headerElement0 += "▼";
        //order hostname desc
        break;

    case orderBy::MACAsc:
        this->headerElement1.pop_back();
        this->headerElement1 += "▲";
        //order ip asc
        break;
    
    case orderBy::MACDesc:
        this->headerElement1.pop_back();
        this->headerElement1 += "▼";
        //order ip desc
        break;

    case orderBy::IPAsc:
        this->headerElement2.pop_back();
        this->headerElement2 += "▲";
        //order mac asc
        break;

    case orderBy::IPDesc:
        this->headerElement2.pop_back();
        this->headerElement2 += "▼";
        //order mac desc
        break;

    case orderBy::StatusAsc:
        this->headerElement3.pop_back();
        this->headerElement3 += "▲";
        //order by status asc
        break;

    case orderBy::StatusDesc:
        this->headerElement3.pop_back();
        this->headerElement3 += "▼";
        //order status desc
        break;    
    default:
        throw std::runtime_error("Estado do ordenamento não é esperado. (currentOrder)");
    }

    this->previousOrder = this->currentOrder;
}
