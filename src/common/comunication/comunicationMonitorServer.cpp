#include "comunication.hpp"

using namespace ss::network;

bool Comunication<monitorServer>::IsAwake(computer& toPC)
{
    logger::GetInstance().Log(__PRETTY_FUNCTION__, "Enviando mensagem de ISAWAKE para o computador " + toPC.GetName());

    Comunication<server>& comServ = Comunication<server>::GetInstance();

    comServ.SendToQueue(toPC, monitorClient, ComunicationPacket::message::ISAWAKE, monitorServer);

    return WaitIsAwakeResponse(toPC, timeout);
}

bool Comunication<monitorServer>::WaitIsAwakeResponse(computer& toPC, short timeout)
{
    Comunication<server>& comServ = Comunication<server>::GetInstance();

    auto initTime = std::chrono::system_clock::now();

    while(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - initTime).count() < timeout)
    {
        try
        {
            auto packet = comServ.ReadFromQueue(monitorServer, toPC);

            if(packet.GetPacketMessage() == packet::packetMesg::IMAWAKE)
            {
                comServ.ClearMessagesTo(monitorServer);

                logger::GetInstance().Log(__PRETTY_FUNCTION__, "Computador " + toPC.GetName() + " está acordado!");

                return true;
            }
        }
        catch(const ComunicationException& e)
        {
            if(e.type == ComunicationException::Type::NOT_FOUND)
            {
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

