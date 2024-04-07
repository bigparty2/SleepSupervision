#ifndef MONITOR_HPP
#define MONITOR_HPP

namespace ss
{
    namespace monitor
    {
        
        class MonitorSubservice
        {
            public:
            
            MonitorSubservice();
            ~MonitorSubservice();
            
            void Start(bool isServer = false);
            void Stop();

            private:

            void clientRun();
            void serverRun();
        };
    }
}

#endif //MONITOR_HPP