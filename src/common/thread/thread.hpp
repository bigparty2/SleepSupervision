#ifndef THREAD_HPP
#define THREAD_HPP

#include <chrono>
#include <thread>

namespace ss
{
    /**
     * @brief Classe com funções uteis relacionadas a threads.
     */
    class thread
    {
        public:

        /**
         * @brief Pausa a execução da thread atual por um determinado número de milissegundos.
         * 
         * @param milliseconds O número de milissegundos para pausar a thread.
         */
        static void Sleep(int milliseconds);
    };
}

#endif //THREAD_HPP