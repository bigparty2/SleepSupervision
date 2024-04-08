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
// #include "../../common/types.hpp"
#include "../../common/computer/computer.hpp"
#include "../../common/MAC/mac.hpp"
#include "../../common/IPV4/ipv4.hpp"

namespace ss
{
    namespace manager
    {
        class computersManager
        {
            public:

            computersManager();
            ~computersManager();

            //Insere na lista os dados de um computador
            void Insert(computer computer);

            //Atualiza os dados de um computador
            void Update(computer computer);

            //Remove um computador da lista
            bool Remove(network::MAC macAddr);
            bool Remove(network::IPV4 ipv4);
            bool Remove(std::string hostname);

            //Retorna a lista de computadores
            computers Get() const;

            //Retorna tempo da ultima atualização
            uint64_t LastUpdate() const;

            //Trata requisições dos processos
            void HandleRequest();

            // Inserção direta na lista de computadores (PARA TESTES)
            void __Insert(computer computer);

            private:

            //Responde a uma chamada da função Get() para comunicação entre processos
            void GetResponse();

            //
            void InsertResponse();

            //
            void UpdateResponse();

            //
            void RemoveResponse();

            //Atualiza a data da ultima atualização
            void UpdateLastUpdate();

            //Lê da area memória comaprtihlada
            computer ReadFromSA() const;

            //Escreve na area de memória compartilhada
            void WriteOnSA(computer pcd);

            //Procura um pc na lista de pcs
            uint64_t IndexOf(std::string hostname);
            uint64_t IndexOf(network::IPV4 ipv4);
            uint64_t IndexOf(network::MAC macAddr);

            //Remove um elemento a partir do index
            void RemoveByIndex(uint64_t index);

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

            // Variaveis de acesso publico para manipucao da lista de computadores
            public:
            
            //Controle de adição de um novo computador
            static constexpr char* NEW_PC  = "NEW_PC";              //adicionar novo computador
            static constexpr char* NEW_PC_NEXT  = "NEW_PC_NEXT";    //Proxima informação
            static constexpr char* NEW_PC_END  = "NEW_PC_END";      //Fim da transmissão

            //Controle de remoção de um computador
            static constexpr char* RM_PC  = "RM_PC";                //Remover um computador
            static constexpr char* RM_PC_NEXT  = "RM_PC_NEXT";      //Proxima informação
            static constexpr char* RM_PC_END  = "RM_PC_END";        //Fim da transmissão

            //Controle de atualização de um computador
            static constexpr char* UPD_PC  = "UPD_PC";              //Atualizar um computador
            static constexpr char* UPD_PC_NEXT  = "UPD_PC_NEXT";    //Proxima informação
            static constexpr char* UPD_PC_END  = "UPD_PC_END";      //Fim da transmissão
        };      
    }       
}

#endif //MANAGER_HPP