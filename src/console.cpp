#include <iostream>
#include <fstream>
#include <chrono>

#include <spdlog/spdlog.h>

#include "networking/tcp_server.h"

void wait_for_input()
{
    std::getchar();
}

int main()
{
    spdlog::set_level(spdlog::level::trace);
    networking::tcp_server server("127.0.0.1", 9876);
    if (server.start())
    {
        
    }

    std::thread t([&]()
        {
            wait_for_input();
        });
    t.join();
}
