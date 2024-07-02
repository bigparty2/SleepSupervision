cmake_minimum_required(VERSION 3.10)
project(SleepSupervision)

#versão GCC 20
set(CMAKE_CXX_STANDARD 20)

# Adicione as flags -Wall e -Wextra
add_compile_options(-Wall -Wextra)

# Adicione todos os arquivos .cpp na pasta src/common
file(GLOB COMMON_SRC "src/common/*.cpp" "src/common/*/*.cpp")

# Adicione todos os arquivos .cpp na pasta src/service
file(GLOB SERVICE_SRC "src/service/*.cpp" "src/service/*/*.cpp")

# Crie uma biblioteca estática com os arquivos .cpp em src/common e src/service
add_library(common_and_service STATIC ${COMMON_SRC} ${SERVICE_SRC})

# Adicione o diretório src ao caminho de inclusão
include_directories(src)

# Adicione o executável do projeto
add_executable(SS src/main.cpp)

# Vincule a biblioteca estática ao executável do projeto
target_link_libraries(SS common_and_service)

#Variaveis globais pré-processador
add_definitions(-DDEBUG=1)
