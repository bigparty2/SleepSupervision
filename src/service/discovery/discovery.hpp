#ifndef DISCOVERY_HPP
#define DISCOVERY_HPP

namespace ss
{
    namespace discovery
    {
        
        class DiscoverySubservice
        {
            public:
            
            DiscoverySubservice();
            ~DiscoverySubservice();
            
            void Start(bool isServer = false);
            void Stop();

            private:

            void clientRun();
            void serverRun();
        };
    }
}

#endif //DISCOVERY_HPP