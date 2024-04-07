#ifndef DEFINES_HPP
#define DEFINES_HPP

//pipes

//PIPES nomeados
#define PIPE_DISCOVERY_TO_INTERFACE "PIPE_DISCOVERY_TO_INTERFACE"   //pipe para o discovery service enviar mensagens para a interface
#define PIPE_INTERFACE_TO_DISCOVERY "PIPE_INTERFACE_TO_DISCOVERY"   //pipe para a interface enviar mensagens para o discovery service
#define PIPE_MONITOR_TO_INTERFACE "PIPE_MONITOR_TO_INTERFACE" //pipe para o monitoring service enviar mensagens para a interface
#define PIPE_INTERFACE_TO_MONITOR "PIPE_INTERFACE_TO_MONITOR" //pipe para a interface enviar mensagens para o monitoring service

//Controles
#define DISCOVERY_TO_INTERFACE 0
#define INTERFACE_TO_DISCOVERY 1
#define MONITORING_TO_INTERFACE 2
#define INTERFACE_TO_MONITORING 3

//Leitura e escrita pipe
#define RW 0666

#endif //DEFINES_HPP