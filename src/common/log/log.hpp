#ifndef LOG_HPP
#define LOG_HPP

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
    // Gerenciador de log
    class logger
    {
        private:

        // Desabilita cópia da classe
        logger(logger &other) = delete;
        void operator=(const logger &) = delete;

        // Constructor
        logger(std::string fileName = "sleepsupervison.log");

        // Instancia do arquivo de log
        std::ofstream logFile;

        // Mutex para garantir que apenas há uma escrita em arquivo por vez
        std::mutex writeMtx;

        // Função para escrita em arquivo
        void write(std::string typeStr, std::string origin, std::string message);

        // Pegar a data atual do sistema
        std::string GetDate();

        public:

        // Pegar instancia da classe
        static logger& GetInstance();

        // Destructor
        ~logger();

        // Inclusão de registro de atividade no log
        void Log(std::string origin, std::string message);

        // Inclusão de registro de aviso no log
        void Warning(std::string origin, std::string message);

        // Inclusão de registro de erro no log
        void Error(std::string origin, std::string message);
    }; 
}

#endif //LOG_HPP