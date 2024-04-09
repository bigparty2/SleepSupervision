# cmake_minimum_required(VERSION 3.10)
# project(SleepSupervison)

# # Encontre os arquivos fonte dentro da pasta src
# file(GLOB SOURCES "src/*.cpp" "src/*/*.cpp" "src/*/*/*.cpp" "src/*/*/*/*.cpp")

# # Adicione o comando add_executable com a lista de arquivos fonte
# add_executable(SleepSupervison ${SOURCES})

# # Inclua diretórios de header
# target_include_directories(SleepSupervison PUBLIC "src")

# # flags
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall -Wextra")


cmake_minimum_required(VERSION 3.10)
project(SleepSupervison)

set(CMAKE_CXX_STANDARD 20)

# Adicione todos os arquivos .cpp na pasta src/common
file(GLOB COMMON_SRC "src/common/*.cpp" "src/common/*/*.cpp")

# Adicione todos os arquivos .cpp na pasta src/service
file(GLOB SERVICE_SRC "src/service/*.cpp" "src/service/*/*.cpp")

# Crie uma biblioteca estática com os arquivos .cpp em src/common e src/service
add_library(common_and_service STATIC ${COMMON_SRC} ${SERVICE_SRC})

# Adicione o diretório src ao caminho de inclusão
include_directories(src)

# Adicione o executável do projeto
add_executable(SleepSupervison src/main.cpp)

# Vincule a biblioteca estática ao executável do projeto
target_link_libraries(SleepSupervison common_and_service)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall -Wextra")

