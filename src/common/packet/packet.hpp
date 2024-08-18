#ifndef PACKET_HPP
#define PACKET_HPP

#include "../types.hpp"
#include "../computer/computer.hpp"
#include <cstdint>

namespace ss
{
    namespace network
    {
        /**
         * @brief Classe que representa um pacote de rede.
         * 
         * A classe packet é responsável por encapsular os dados de um pacote de rede.
         * Ela possui métodos para obter e definir os dados do pacote, além de verificar se os dados foram inicializados corretamente.
         */
        class packet
        {
            public:

            /**
             * @brief Estrutura que define a estrutura de um pacote de rede.
             * 
             * A estrutura _packet contém os campos que compõem um pacote de rede, como número de sequência, endereço MAC de origem, endereço IPv4 de origem, etc.
             */
            
            
            // struct _packet
            // {
            //     byte seqNum;
            //     byte macOrigin[6];
            //     int32_t ipv4Origin;
            //     byte nameOrigin[64];
            //     uint16_t portOrigin;
            //     int64_t timestamp;
            //     byte message;
            // };

            struct _packet
            {
                byte seqNum; /**< Número de sequência do pacote. */
                computer::computerData pcOrigin; /**< Dados do computador de origem do pacote. */
                uint16_t portOrigin; /**< Porta de origem do pacote. */
                int64_t timestamp; /**< Timestamp do pacote. */
                byte message; /**< Mensagem do pacote. */
                computer::computerData payload; /**< Dados do payload do pacote. */
            };

            /**
             * @brief Enumeração que define os tipos de mensagem do pacote.
             */
            enum packetMesg
            {
                REGITRY = 0, /**< Registro. */
                EXIT = 1, /**< Saída. */
                OK = 2, /**< OK. */
                ERROR = 3, /**< Erro. */
                ISAWAKE = 4, /**< Está acordado. */
                IMAWAKE = 5, /**< Estou acordado. */
                EMPTY
            };

            /**
             * @brief Construtor padrão da classe packet.
             */
            packet(){};

            /**
             * @brief Construtor da classe packet que recebe um pacote como parâmetro.
             * 
             * @param packet O pacote a ser atribuído à classe.
             */
            packet(_packet packet);

            /**
             * @brief Construtor da classe packet que recebe informações para criar um pacote.
             * 
             * @param computer O computador de origem do pacote.
             * @param message A mensagem do pacote.
             * @param port A porta de origem do pacote.
             * @param seq O número de sequência do pacote.
             */
            packet(computer computer, packetMesg message, uint16_t port, byte seq = 1);

            /**
             * @brief Destrutor da classe packet.
             */
            ~packet();

            /**
             * @brief Obtém os dados do pacote.
             * 
             * @return Um ponteiro para os dados do pacote.
             */
            byte* GetPacketData();

            /**
             * @brief Obtém o pacote.
             * 
             * @return O pacote.
             */
            _packet GetPacket();

            /**
             * @brief Obtém o tamanho do pacote.
             * 
             * @return O tamanho do pacote.
             */
            static size_t GetPacketSize();

            /**
             * @brief Define os dados do pacote.
             * 
             * @param computer O computador de origem do pacote.
             * @param message A mensagem do pacote.
             * @param port A porta de origem do pacote.
             * @param seq O número de sequência do pacote.
             */
            void SetPacket(computer computer, packetMesg message, uint16_t port, byte seq = 1);

            /**
             * @brief Define o pacote.
             * 
             * @param packet O pacote a ser definido.
             */
            void SetPacket(_packet packet);

            /**
             * @brief Verifica se os dados do pacote foram inicializados corretamente.
             * 
             * @return true se os dados foram inicializados corretamente, false caso contrário.
             */
            bool IsDataInicialized();

            private:

            bool dataInicialized = false; /**< Flag que indica se os dados do pacote foram inicializados corretamente. */

            _packet packetData; /**< Dados do pacote. */

            byte* packetPointerData; /**< Ponteiro para os dados do pacote. */

            /**
             * @brief Obtém o timestamp atual.
             * 
             * @return O timestamp atual.
             */
            static int64_t GetTimestamp();
        };
    }
}   

#endif //PACKET_HPP
