cmake_minimum_required(VERSION 3.10)
project(SleepSupervisor)

# Encontre os arquivos fonte dentro da pasta src
file(GLOB SOURCES "src/*.cpp" "src/*/*.cpp" "src/*/*/*.cpp")

# Adicione o comando add_executable com a lista de arquivos fonte
add_executable(SleepSupervisor ${SOURCES})

# Inclua diretórios de header
target_include_directories(SleepSupervisor PUBLIC "src")

# flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall -Wextra")
