#ifndef COMUNICATION_TYPE_HPP
#define COMUNICATION_TYPE_HPP

namespace ss
{
    namespace network
    {
        enum ComunicationType
        {
            monitorServer,
            monitorClient,
            discoveryServer,
            discoveryClient,
            managerServer,
            managerClient,
            bully,
            server
        };
    }
}

#endif // COMUNICATION_TYPE_HPP