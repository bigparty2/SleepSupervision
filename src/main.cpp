#include "service/discovery/discovery.hpp"
#include "service/interface/interface.hpp"
#include "service/manager/manager.hpp"
#include "service/monitor/monitor.hpp"
#include "common/logger/logger.hpp"
#include "common/computer/computer.hpp"
#include "service/network/MAC/mac.hpp"
#include <iostream>


int main (int argc, char** argv)
{
    auto pc = ss::computer();

    pc.GetComputerInfo();

    return (0x0);
}