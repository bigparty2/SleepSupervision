# SleepSupervison 1

O projeto SleepSupervision é um sistema de gerenciamento de computadores. Ele monitora o estado de atividade dos computadores gerenciados, identificando se estão em modo de suspensos ou ativos. Além disso, o sistema tem a capacidade de acordar computadores que estão em hibernação ou desligados, utilizando o protocolo WakeOnLan (WoL).

## Como Compilar e Executar

1. Na pasta raiz, crie uma pasta de nome 'build' executando `mkdir build`;
2. Navegue até a pasta build com `cd build`;
3. Configure o Cmake executando `cmake ..`;
4. Compile e execute o software com `make M` para executar o software como Gerenciador e `make P` para executar o software como Participante.

## Referências

- https://www.usenix.org/conference/nsdi12/technical-sessions/presentation/sen
- https://www.microsoft.com/en-us/research/project/greenup/
