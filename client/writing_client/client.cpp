#include <iostream>
#include <api/example_body.h>

#include <spdlog/spdlog.h>

#include <networking/tcp_client.h>

#include "api/message_header.h"
#include "api/subscribe_message.h"

void keep_running()
{
    std::getchar();
}

int main()
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    spdlog::set_level(spdlog::level::trace);

    networking::tcp_client client("127.0.0.1", 9876);
    client.message_received = [&]()
    {
        if (client.message_queue->size_approx() > 0)
        {
            spdlog::debug("[client] we received a message");
        }
    };

    const auto topic_str = std::string("aaa");
    const auto service_str = std::string("service");
    api::message_header header;
    std::copy(topic_str.begin(), topic_str.end(), header.topic.begin());
    std::copy(service_str.begin(), service_str.end(), header.service_name.begin());

    api::example_body example_body{ 69, 420 };

    std::vector<unsigned char> body_bytes(sizeof(api::example_body));
    memcpy(&body_bytes[0], &example_body, sizeof(api::example_body));

    for (auto i = 0; i < 10; i++)
    {
        client.send_message(header, body_bytes);
    }

    std::thread t([&]()
        {
            keep_running();
        });
    t.join();
}