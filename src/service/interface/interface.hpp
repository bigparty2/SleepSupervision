#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <thread>
#include <unistd.h>
#include <locale>
#include <mutex>
#include <cmath>
#include <errno.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/poll.h>
#include "../manager/manager.hpp"
#include "../../common/types.hpp"
#include "../../common/thread/thread.hpp"
#include "../../common/wakeOnLan/wakeOnLan.hpp"

namespace ss
{
    namespace interface
    {
        /**
         * Gerenciador da interface do usuário.
         */
        class interfaceManager
        {
            public:

            /**
             * Gerenciador da resolução do terminal.
             */
            class terminalSizeManager
            {
                public:

                terminalSizeManager();
                ~terminalSizeManager(){};

                /**
                 * Retorna a resolução do terminal da última leitura.
                 * @return As coordenadas (x, y) da resolução do terminal.
                 */
                coord Get();

                /**
                 * Captura a resolução atual do terminal e retorna um booleano informando 
                 * se houve ou não alteração na resolução do terminal desde a última leitura.
                 * @return true se houve alteração na resolução do terminal, false caso contrário.
                 */
                bool HasChange();

                private:

                /**
                 * Atualiza a resolução (em "celulas") atual do terminal.
                 */
                void UpdateCurrentRes();

                // Armazenamento da resolução atual e a resolução anterior
                ss::coord resAtual = {0, 0};
                ss::coord resAnterior = {0, 0}; 
            };

            // constructor
            interfaceManager(){};
            interfaceManager(ss::manager::computersManager &cm, bool manager);
            
            // destructor
            ~interfaceManager();

            /**
             * Inicializa o serviço.
             * @param cm O gerenciador de computadores.
             * @param manager Indica se é um gerenciador ou participante.
             */
            void Init(ss::manager::computersManager &cm, bool manager);

            /**
             * Encerra o serviço.
             */
            void End();

            /**
             * Aguarda o encerramento do processo.
             */
            void Join();

            /**
             * Limpa a tela.
             */
            static void Clear();

            /**
             * Retorna a posição atual do cursor.
             * @return As coordenadas (x, y) da posição do cursor.
             */
            static coord GetCursorPosition();

            /**
             * Posiciona o cursor na coordenada informada.
             * @param y A coordenada y.
             * @param x A coordenada x.
             */
            static void GotoYX(int y, int x);

            /**
             * Oculta o cursor.
             */
            static void HideCursor();

            /**
             * Restaura o cursor.
             */
            static void ShowCursor();

            /**
             * Bloqueia a rolagem de tela.
             */
            static void LockScrolling();

            /**
             * Desbloqueia a rolagem de tela.
             */
            static void UnlockScrolling();

            /**
             * Define letra cor preta com fundo branco.
             */
            static void SetTextBlackBackgroundWrite();

            /**
             * Restaura as cores padrões do terminal.
             */
            static void SetColorDefault();

            private:

            /**
             * Gerenciador de entradas do usuário.
             */
            void InputManager();

            /**
             * Gerenciador das exibições em tela.
             */
            void OutputManager();

            /**
             * Exibe os dados em tela.
             */
            void Draw();

            /**
             * Print do frame.
             */
            void PrintFrame();

            /**
             * Print do rodapé.
             */
            void PrintFooter();

            /**
             * Print cabeçalho.
             */
            void PrintHeader();

            /**
             * Print da tabela.
             */
            void PrintTable();

            /**
             * Print do número das páginas.
             */
            void PrintPagesInfo();

            /**
             * Detecta input aguardando no buffer.
             * @return true se houver input no buffer, false caso contrário.
             */
            bool KbHit();

            /**
             * Sessão crítica.
             */
            void CriticalRegion();
            
            /**
             * Ajusta o método de ordenamento.
             */
            void AdjustOrdering();

            /**
             * Lista de teclas capturadas pelo sistema.
             * @param key A tecla pressionada.
             * @return true se a tecla é aceita, false caso contrário.
             */
            bool AcceptedKeys(int key);

            /**
             * Retorna os computadores a serem exibidos na tela.
             * @return A lista de computadores a serem exibidos.
             */
            computers PcsOfCurrentPage();

            /**
             * Definição do ordenamento.
             */
            enum orderBy
            {
                hostnameAsc,
                hostnameDesc,
                MACAsc,
                MACDesc,
                IPAsc,
                IPDesc,
                StatusAsc,
                StatusDesc
            };

            // Variáveis para controle do método de ordenamento
            orderBy currentOrder;
            orderBy previousOrder;

            // Define um tempo padrão para pausas em threads
            const uint32_t THREAD_SLEEP_TIME = 125;

            // Controle para execução das threads
            bool threadKeepAlive = true;

            // Controle para execução do serviço
            bool isRunning = false;

            // Ponteiro para a estrutura da Management Service
            computers machines;

            // Instância do gerenciador de computadores
            ss::manager::computersManager *machinesManager = nullptr;

            // Gerenciador da última atualização da lista de participantes
            uint64_t lastChange;

            // Instância do gerenciador da resolução do terminal
            interfaceManager::terminalSizeManager *res = nullptr;

            // Mutex de proteção ao acesso concorrente
            std::mutex iMutex;

            // Controle utilizado pelo gerenciador de input para indicar ao gerenciador de output 
            // quando há necessidade de redesenhar o conteúdo na tela 
            bool inputRedraw = false;  

            // Controle de participante ou gerenciador
            bool IsManager;

            // Backup das configurações do terminal
            termios oldTermAttr;

            // Thread de entrada
            std::thread tin;

            // Thread de saída
            std::thread tout;

            // Thread de controle de encerramento
            std::thread tend;

            // Nome do host do computador local
            computer localComputer;

            // Controle de seleção
            short selection = -1;

            // Último registro do número de computadores registrados
            // Obs.: Em caso de mudança, a seleção retorna para o elemento 0
            size_t lastPCsSize = 0;

            // Lista de teclas aceitas
            std::vector<int> keysList = {(int)'Q', (int)'q', 27, 32};

            // Variáveis de controle e posicionamentos na interface
            const unsigned short frameTopLinePos = 5;
            unsigned short frameBottomLinePos;
            unsigned short frameRightColumnPos;
            const unsigned short frameMargin = 2;
            const unsigned linesAfterEndFrame = 5;
            unsigned short commandInfoLinePos;
            unsigned short headerLinePos;
            unsigned short headerElementLenght;
            unsigned short headerElement0Pos;
            unsigned short headerElement1Pos;
            unsigned short headerElement2Pos;
            unsigned short headerElement3Pos;
            unsigned short rowsPerPage;
            unsigned short CurrentPage = 1;
            unsigned short numOfPages;
            unsigned short firstRowPos;
            unsigned short currentRows;

            // Constantes de elementos textuais da interface gráfica
            const std::string title = "Sleep Supervision";
            const std::string tableTitle = "Computadores conectados ao sistema";
            const std::string noDataMsg = "Não há computadores identificados no sistema";
            const std::string hostMsg = "Dados do host: ";
            const std::string clientWaittMsg = "A procura de um host para se conectar...";
            const std::string frameTopLeft = "╔";
            const std::string frameHorizontal = "═";
            const std::string frameTopRight = "╗";
            const std::string frameVetical = "║";
            const std::string frameBottomLeft = "╚";
            const std::string frameBottomRight = "╝";
            const std::string tableCommand = "Acordar (Espaço) │ Ajuda (H) │ Sair (Q)";
            const std::string headerCommand = "Ordenar por (Espaço) │ Ajuda (H) │ Sair (Q)";
            const std::string noDataCommand = "Ajuda (H) │ Sair (Q)";
            
            // Elementos textuais não constantes pois podem sofrer alterações (ordenamento por coluna)
            std::string headerElement0 = "Hostname▲";
            std::string headerElement1 = "MAC ";
            std::string headerElement2 = "IP ";
            std::string headerElement3 = "Status ";
        };
    }
}

#endif // INTERFACE_HPP
