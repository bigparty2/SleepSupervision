#ifndef MANAGER_HPP
#define MANAGER_HPP

#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../common/computer/computer.hpp"
#include "../../common/MAC/mac.hpp"
#include "../../common/IPV4/ipv4.hpp"
#include "../../common/thread/thread.hpp"
#include "../../common/socket/socket.hpp"

namespace ss
{
    namespace manager
    {
        /**
         * @brief Classe para gerenciamento de computadores.
         * 
         * A classe `computersManager` é responsável por gerenciar os computadores da rede.
         * Ela permite inserir, atualizar e remover computadores da lista, bem como obter a lista de computadores.
         * 
         * A classe também permite tratar requisições dos processos, como inserção, atualização e remoção de computadores.
         */
        class computersManager
        {
            public:

            computersManager(bool isHost);
            ~computersManager();

            //Insere na lista os dados de um computador
            void Insert(computer computer);

            //Atualiza os dados de um computador
            void Update(computer computer);

            //Remove um computador da lista
            void Remove(computer computer);
            // bool Remove(network::MAC macAddr);
            // bool Remove(network::IPV4 ipv4);
            // bool Remove(std::string hostname);

            //Retorna a lista de computadores
            computers Get() const;

            //Retorna tempo da ultima atualização
            uint64_t LastUpdate() const;

            //Trata requisições dos processos
            void HandleRequest();

            // Inserção direta na lista de computadores (PARA TESTES)
            void __Insert(computer computer);

            //Instancia com dados deste computador
            computer thisComputer;

            //Retorna o computador host
            computer GetHost();

            //Define o computador host
            void SetHost(computer computer);

            // Remove  o computador host
            void ClearHost();

            //Retorna a informação se o computador host está definido
            bool IsHostSeted() const;

            private:

            /// @brief Variavel para validar se o computador é host
            bool isHost;
          
            //Dados do computador host
            //Obs.: Como os dados do computador host são adquiridos após a separação dos processos
            //este deve ser atribuido por memoria compartilhada para que o processo que gerencia a
            //lista de computadores possa definir o host e fornece-lo para os demais processos.
            computer* hostComputer;

            //Responde a uma chamada da função Get() para comunicação entre processos
            void GetResponse();

            //
            void InsertResponse();

            //
            void UpdateResponse();

            //Responde a chamada de remoção de um computador (Executado apenas pelo computador host)
            void RemoveResponse();

            /// @brief Envia para o discovery do computador host uma mensagem de saída do sistema
            void SendExitMessage(computer computer);

            /// @brief Responde a chamada de pegar os dados do computador host
            void GetHostResponse();

            /// @brief Responde a chamada de definir o computador host
            void SetHostResponse();

            /// @brief Responde a chamada de limpeza do host
            void ClearHostResponse();

            //Atualiza a data da ultima atualização
            void UpdateLastUpdate();

            //Lê da area memória comaprtihlada
            computer ReadFromSA() const;

            //Escreve na area de memória compartilhada
            void WriteOnSA(computer pcd);

            //Procura um pc na lista de pcs
            uint64_t IndexOf(computer computer);
            uint64_t IndexOf(std::string hostname);
            uint64_t IndexOf(network::IPV4 ipv4);
            uint64_t IndexOf(network::MAC macAddr);

            //Remove um elemento a partir do index
            void RemoveByIndex(uint64_t index);

            //IPCControl to string
            std::string IPCControlToString() const;
            static std::string IPCControlToString(int control);

            //Pegar o número de um novo ID para um computador que está ingressando
            int GetNewID();

            //Armazena as informações dos computadores
            computers _data;

            //Posição definida para indicar falha em busca de index
            const static uint64_t npos = (uint64_t)-1;

            //Semáforo para controle de acesso à região critica
            sem_t* sem = nullptr;

            //Endereços de memória compartilhada (sa = Shared Address)
            void* saHostname = nullptr;     //string
            void* saMAC = nullptr;          //char[6]
            void* saIPV4 = nullptr;         //int
            void* saStatus = nullptr;       //int

            //Controle de mudanças nos dados
            void* saLastUpdate = nullptr;   //uint64

            //Variavel de constrole para troca de informações entre processos
            void* saIPCControl = nullptr;    //int

            void* saIsHostSeted = nullptr;

            static constexpr char* SEM_NAME = "/SS_COMPUTERSMANAGER_SEM";

            //Definições para variavel de controle de comunicação entre processos
            static constexpr int INSERT  = 0;   //Inserção de dado
            static constexpr int UPDATE  = 1;   //Atualização de dado
            static constexpr int GET     = 3;   //Captura dos dados
            static constexpr int READY   = 4;   //Pronto para nova ação
            static constexpr int WAIT    = 5;   //Aguardando retorno
            static constexpr int NEXT    = 6;   //Proximo dado
            static constexpr int END     = 7;   //Fim da ação
            static constexpr int ENDLIST = 8;   //Fim da lista
            static constexpr int SIZE    = 9;   //Quantidade de elementos
            static constexpr int REMOVE  = 10;  //Remover elemento 
            static constexpr int GETHOST = 11;  //Retornar host
            static constexpr int SETHOST = 12;  //Definir host
            static constexpr int ISSETED = 13;  //Verificar se host está definido
            static constexpr int YES     = 14;  //Indica que host foi definido
            static constexpr int NO      = 15;  //Indica que host não foi definido
            static constexpr int RMHOST  = 16;  //Remover/Limpar host

            static const uint16_t DISCOVERY_PORT_SERVER = 45001;
            static const uint16_t MANAGER_PORT_CLIENT_INIT = 45105;
            static const uint16_t MANAGER_PORT_CLIENT_END = 45205;
        };      
    }       
}

#endif //MANAGER_HPP