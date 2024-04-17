// TODO: Adaptar classe para trabalhar com semaforos para que seja process safe

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <mutex>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <algorithm>

namespace ss
{
    /**
     * @brief Classe responsável por registrar atividades, avisos e erros em um arquivo de log.
     */
    class logger
    {
        private:

        // Desabilita cópia da classe
        logger(logger &other) = delete;
        void operator=(const logger &) = delete;

        /**
         * @brief Construtor da classe logger.
         * @param fileName O nome do arquivo de log. O valor padrão é "sleepsupervison.log".
         */
        logger(std::string fileName = "sleepsupervison.log");

        // Instancia do arquivo de log
        std::ofstream logFile;

        // Mutex para garantir que apenas há uma escrita em arquivo por vez
        std::mutex writeMtx;

        /**
         * @brief Função para escrever no arquivo de log.
         * @param typeStr O tipo de registro (atividade, aviso ou erro).
         * @param origin A origem do registro.
         * @param message A mensagem do registro.
         */
        void write(std::string typeStr, std::string origin, std::string message);

        /**
         * @brief Obtém a data atual do sistema.
         * @return A data atual no formato "dd/mm/aaaa".
         */
        std::string GetDate();

        public:

        /**
         * @brief Obtém a instância da classe logger.
         * @return A instância da classe logger.
         */
        static logger& GetInstance();

        /**
         * @brief Destrutor da classe logger.
         */
        ~logger();

        /**
         * @brief Registra uma atividade no arquivo de log.
         * @param origin A origem da atividade.
         * @param message A mensagem da atividade.
         */
        void Log(std::string origin, std::string message);

        /**
         * @brief Registra um aviso no arquivo de log.
         * @param origin A origem do aviso.
         * @param message A mensagem do aviso.
         */
        void Warning(std::string origin, std::string message);

        /**
         * @brief Registra um erro no arquivo de log.
         * @param origin A origem do erro.
         * @param message A mensagem do erro.
         */
        void Error(std::string origin, std::string message);
    }; 
}

#endif //LOGGER_HPP