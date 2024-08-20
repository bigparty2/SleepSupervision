/**
 * @file main.cpp
 * @brief Arquivo principal do programa SleepSupervison.
 * 
 * Este arquivo contém a função principal `main` que inicia o programa SleepSupervison.
 * O programa é responsável por supervisionar o sono de um computador e gerenciar os participantes.
 * Ele possui três subserviços: Discovery, Monitor e Interface.
 * O subserviço de Discovery é responsável por descobrir os participantes na rede.
 * O subserviço de Monitor é responsável por monitorar o sono dos participantes.
 * O subserviço de Interface é responsável por fornecer uma interface para interagir com o programa.
 * O programa pode ser iniciado como servidor (M) ou como participante (P), dependendo do argumento passado na linha de comando.
 * 
 * @param argc O número de argumentos passados na linha de comando.
 * @param argv Um array de strings contendo os argumentos passados na linha de comando.
 * @return O código de saída do programa.
 */
#include "service/discovery/discovery.hpp"
#include "service/interface/interface.hpp"
#include "service/manager/manager.hpp"
#include "service/monitor/monitor.hpp"
#include "common/logger/logger.hpp"
#include "common/computer/computer.hpp"
#include "common/MAC/mac.hpp"
#include "common/comunication/comunication.hpp"
#include "common/comunication/comunicationType.hpp"
#include <sys/prctl.h>
#include <sys/wait.h>
#include <iostream>


int main (int argc, char** argv)
{
    ss::logger::GetInstance().Log(__PRETTY_FUNCTION__, "Iniciando ...");

    //Define o nome do processo
    prctl(PR_SET_NAME, "SS");

    //verificação da quantidade de argumentos
    // if(argc != 2)
    // {
    //     printf("Quantidade de argumentos invalidos!\n");
    //     ss::logger::GetInstance().Error(__PRETTY_FUNCTION__, "Quantidade de argumentos invalidos!");
    //     return EXIT_FAILURE;
    // }

    //Modo de inicialização: M ou P
    bool isManager = false;

    //verificação para o argumento passado
    // {
    //     std::string argStr(argv[1]);

    //     if(argStr == "P" or argStr == "p")
    //     {
    //         isManager = false;
    //         ss::logger::GetInstance().Log(__PRETTY_FUNCTION__, "Modo de inicialização: Participante");
    //     }
    //     else if(argStr == "M" or argStr == "m")
    //     {
    //         ss::logger::GetInstance().Log(__PRETTY_FUNCTION__, "Modo de inicialização: Gerenciador");
    //         isManager = true;
    //     }
    //     else
    //     {
    //         ss::logger::GetInstance().Error(__PRETTY_FUNCTION__, "Argumento inválido!");
    //         printf("Argumento \"%s\" invalido!\n", argStr.c_str());
    //         return EXIT_FAILURE;
    //     }
    // }

    ss::logger::GetInstance().Log(__PRETTY_FUNCTION__, "Iniciado subserviço Manager");

    //gerenciador de participantes compartilhado
    ss::manager::computersManager cm(isManager);

    // ss::logger::GetInstance().Log(__PRETTY_FUNCTION__, "Iniciando gerenciador de mensagens (Comunication)");

    // Inicializacao do servidor da classe comunication
    // ss::network::Comunication<ss::network::ComunicationType::server>& 
    //     comServer = ss::network::Comunication<ss::network::ComunicationType::server>::GetInstance(&cm);
    // comServer.StartAsync();

    //inicia o subserviço de descoberta
    auto pidDiscovery = fork();
    if(!pidDiscovery)
    {
        ss::logger::GetInstance().Log(__PRETTY_FUNCTION__, "Iniciado subserviço Discovery");
 
        //Define o nome do processo
        prctl(PR_SET_NAME, "SS_Discovery");

        ss::discovery::DiscoverySubservice ds(cm);

        ds.Start(isManager);

        ss::logger::GetInstance().Log(__PRETTY_FUNCTION__, "Finalizando subserviço Discovery");

        return EXIT_SUCCESS;
    }

    //inicia o subserviço de monitoramento
    auto pidMonitor = fork();
    if(!pidMonitor)
    {
        ss::logger::GetInstance().Log(__PRETTY_FUNCTION__, "Iniciado subserviço Monitor");

        prctl(PR_SET_NAME, "SS_Monitor");

        ss::monitor::MonitorSubservice ms(cm);

        ms.Start(isManager);

        ss::logger::GetInstance().Log(__PRETTY_FUNCTION__, "Finalizando subserviço Monitor");

        return EXIT_SUCCESS;
    }

    //inicia o subserviço de interface
    auto pidInterface = fork();
    if(!pidInterface)
    {
        ss::logger::GetInstance().Log(__PRETTY_FUNCTION__, "Iniciado subserviço Interface");

        bool imLeader = cm.ImHost();

        //Define o nome do processo
        prctl(PR_SET_NAME, "SS_Interface");

        std::cout << "Sleep Supervision" << std::endl;

        while(true)
        {
            //Inicializa o serviço
            ss::interface::interfaceManager* interfaceManager = new ss::interface::interfaceManager(cm, cm.ImHost());

            //Aguarda finalização do serviço        
            interfaceManager->Join();

            if(imLeader == cm.ImHost())
            {
                break;
            }
            else
            {
                imLeader = cm.ImHost();
                delete interfaceManager;
            }
        }

        
        ss::logger::GetInstance().Log(__PRETTY_FUNCTION__, "Finalizando subserviço Interface");

        return EXIT_SUCCESS;
    }
    else
    {
        ss::logger::GetInstance().Log(__PRETTY_FUNCTION__, "Iniciando loop de tratamento de requisições ao manager");

        while(waitpid(pidInterface, NULL, WNOHANG) == 0)
        {
            cm.HandleRequest();
        }
    }

    kill(pidDiscovery, 9);
    kill(pidMonitor, 9);

    // comServer.Stop();

    cm.~computersManager();

    ss::logger::GetInstance().Log(__PRETTY_FUNCTION__, "Encerrando ...");

    return (0x0);
}