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

    // Lista testa
    // for(int i = 1; i < 50; i++)
    // {
    //     ss::computer temp;
    //     temp.name = std::string("Teste com nome maior " + std::to_string(i));
    //     temp.ipv4 = std::string("127.0.0." + std::to_string(i));
    //     temp.macAddr = "12:34:56:78:9A:BC";
    //     temp.status = ss::computer::computerStatus::awake;
    //     cm.__Insert(temp);
    // }

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

    //inicia o subserviço de interface
    auto pidInterface = fork();
    if(!pidInterface)
    {
        //Define o nome do processo
        prctl(PR_SET_NAME, "SS_Interface");

        std::cout << "Sleep Manager" << std::endl;

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
            waitpid(pidInterface, NULL, WUNTRACED); //trava a execução WUNTRACED
        } 
        //waitpid(pidInterface, NULL, WUNTRACED); //trava a execução WUNTRACED
    }

    kill(pidDiscovery, 9);
    // kill(pidMonitoring, 9);

    return (0x0);
}