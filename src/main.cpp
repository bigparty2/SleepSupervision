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
            isManager = false;
        else if(argStr == "M" or argStr == "m")
            isManager = true;
        else
        {
            // TODO: Incluir no LOG
            printf("Argumento \"%s\" invalido!\n", argStr.c_str());
            return EXIT_FAILURE;
        }
    }

    //gerenciador de participantes compartilhado
    ss::manager::computersManager cm;

    //inicia o subserviço de descoberta
    auto pidDiscovery = fork();
    if(!pidDiscovery)
    {
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
        prctl(PR_SET_NAME, "SS_Monitor");

        ss::monitor::MonitorSubservice ms(cm);

        ms.Start(isManager);

        return EXIT_SUCCESS;
    }

    //inicia o subserviço de interface
    auto pidInterface = fork();
    if(!pidInterface)
    {
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