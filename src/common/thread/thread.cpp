#include "thread.hpp"

using namespace ss;

void thread::Sleep(int  milliseconds)
{
    return std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}