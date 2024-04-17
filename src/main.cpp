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
#include <sys/prctl.h>
#include <sys/wait.h>
#include <iostream>


int main (int argc, char** argv)
{
    ss::logger::GetInstance().Log(__PRETTY_FUNCTION__, "SleepManager iniciado");


    //Define o nome do processo
    prctl(PR_SET_NAME, "SS");

    //verificação da quantidade de argumentos
    if(argc != 2)
    {
        printf("Quantidade de argumentos invalidos!\n");
        return EXIT_FAILURE;
    }

    //Modo de inicialização: M ou P
    bool isManager;

    //verificação para o argumento passado
    {
        std::string argStr(argv[1]);

        if(argStr == "P" or argStr == "p")
        {
            isManager = false;
            ss::logger::GetInstance().Log(__PRETTY_FUNCTION__, "iniciando como participante");
        }
        else if(argStr == "M" or argStr == "m")
        {
            ss::logger::GetInstance().Log(__PRETTY_FUNCTION__, "iniciando como servidor");
            isManager = true;
        }
        else
        {
            // TODO: Incluir no LOG
            printf("Argumento \"%s\" invalido!\n", argStr.c_str());
            return EXIT_FAILURE;
        }
    }

    //gerenciador de participantes compartilhado
    ss::manager::computersManager cm(isManager);

    //inicia o subserviço de descoberta
    auto pidDiscovery = fork();
    if(!pidDiscovery)
    {
        ss::logger::GetInstance().Log(__PRETTY_FUNCTION__, "Iniciado processo do Discovery");
 
        //Define o nome do processo
        prctl(PR_SET_NAME, "SS_Discovery");

        ss::discovery::DiscoverySubservice ds(cm);

        ds.Start(isManager);

        return EXIT_SUCCESS;
    }

    //inicia o subserviço de monitoramento
    auto pidMonitor = fork();
    if(!pidMonitor)
    {
        ss::logger::GetInstance().Log(__PRETTY_FUNCTION__, "Iniciado processo do Monitor");

        prctl(PR_SET_NAME, "SS_Monitor");

        ss::monitor::MonitorSubservice ms(cm);

        ms.Start(isManager);

        return EXIT_SUCCESS;
    }

    //inicia o subserviço de interface
    auto pidInterface = fork();
    if(!pidInterface)
    {
        ss::logger::GetInstance().Log(__PRETTY_FUNCTION__, "Iniciado processo da Interface");

        //Define o nome do processo
        prctl(PR_SET_NAME, "SS_Interface");

        std::cout << "Sleep Supervison" << std::endl;

        //Inicializa o serviço
        ss::interface::interfaceManager interfaceManager(cm, isManager);

        //Aguarda finalização do serviço        
        interfaceManager.Join();

        return EXIT_SUCCESS;
    }
    else
    {
        if(isManager)
            while(waitpid(pidInterface, NULL, WNOHANG) == 0)
            {
                cm.HandleRequest();
            }
        else
        {
            //trava a execução WUNTRACED
            waitpid(pidInterface, NULL, WUNTRACED); 
        }
    }

    kill(pidDiscovery, 9);
    kill(pidMonitor, 9);

    return (0x0);
}