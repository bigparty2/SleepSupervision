#include "logger.hpp"

using namespace ss;

logger& logger::GetInstance()
{
    // Instancia unica de classe
    static logger instance;

    return instance;
}

logger::logger(std::string fileName)
{
    // Verifica se arquivo já existe na pasta
    if(!std::filesystem::exists(std::filesystem::path("./" + fileName)))
    {
        // Cria e insere o cabeçalho
        logFile.open(fileName);

        logFile << "Date\tType\tOrigin\tMessage" << std::endl;
    }
    else
    {
        logFile.open(fileName, std::ios::app);    
    }

    // Inicializa semaforo
    sem_unlink(SEM_NAME);
    sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
}

logger::~logger()
{
    logFile.flush();
    sem_close(sem);
    sem_unlink(SEM_NAME);
}

void logger::write(std::string typeStr, std::string origin, std::string message)
{
    // Mensagem não deve conter tabs ou nova linha
    std::replace(message.begin(), message.end(), '\t', ' ');
    std::replace(message.begin(), message.end(), '\n', ' ');

    // std::lock_guard<std::mutex> lock(writeMtx);
    sem_wait(this->sem);

    logFile << GetDate() << "\t" << typeStr << "\t" << origin << "\t" << message << std::endl;

    sem_post(this->sem);
}

std::string logger::GetDate()
{
    std::ostringstream buffer;

    // Obtém a hora atual do sistema
    auto now = std::chrono::system_clock::now();

    // Obtém a parte dos milissegundos
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    // Converte para um objeto time_t
    std::time_t time = std::chrono::system_clock::to_time_t(now);

    // Converte para um objeto tm
    std::tm* localTime = std::localtime(&time);

    // Imprime a data e hora no formato desejado
    buffer << std::put_time(localTime, "%d/%m/%Y %H:%M:%S");

    // Imprime os milissegundos
    buffer << ":" << std::setfill('0') << std::setw(3) << ms.count();

    return buffer.str();
}

void logger::Log(std::string origin, std::string message)
{
    write("Information", origin, message);
}

void logger::Warning(std::string origin, std::string message)
{
    write("Warning", origin, message);
}

void logger::Error(std::string origin, std::string message)
{
    write("Error", origin, message);
}

void ss::logger::Debug(std::string origin, std::string message)
{
    #ifdef DEBUG
        if(DEBUG == 1)
            write("Debug", origin, message);
    #endif
}
