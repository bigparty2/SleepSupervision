#include "comunication.hpp"

using namespace ss::network;

bool Comunication<monitorClient>::ResponseIsAwakeRequest()
{
    Comunication<server>& comServer = Comunication<server>::GetInstance();

    auto initTime = std::chrono::system_clock::now();

    while(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - initTime).count() < timeout)
    {
        try
        {
            auto packet = comServer.ReadFromQueue(monitorClient);

            if(packet.GetPacketMessage() == packet::packetMesg::ISAWAKE)
            {
                comServer.ClearMessagesTo(monitorClient);

                comServer.SendToQueue(std::get<0>(packet.GetOriginPCInfo()), monitorServer, ComunicationPacket::message::IMAWAKE, monitorClient);

                logger::GetInstance().Log(__PRETTY_FUNCTION__, "Respondendo ISAWAKE para o computador " + std::get<0>(packet.GetOriginPCInfo()).GetName());

                return true;
            }
        }
        catch(const ComunicationException& e)
        {
            if(e.type == ComunicationException::Type::NOT_FOUND)
            {
                logger::GetInstance().Log(__PRETTY_FUNCTION__, "Mensagem não encontrada, tentando novamente ...");

                continue;
            }
            else
            {
                throw e;
            }
        }
    }

    return false;
}