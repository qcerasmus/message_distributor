#include <array>

#include <spdlog/spdlog.h>

#include "api/example_body.h"
#include "api/message_header.h"
#include "api/subscribe_message.h"

#include "networking/tcp_client.h"

void keep_running()
{
    std::getchar();
}

int main()
{
    spdlog::set_level(spdlog::level::trace);

    networking::tcp_client client("127.0.0.1", 9876);
    client.message_received = [&]()
    {
        if (client.message_queue->size_approx() > 0)
        {
            networking::message_packet mp{};
            if (client.message_queue->try_dequeue(mp))
            {
                auto *const example_body = reinterpret_cast<api::example_body *>(&mp.body_[0]);
                spdlog::debug("[reading_client] example body received: i = {} j = {}", example_body->i, example_body->j);
            }
        }
    };

    const auto topic_str = std::string("sub");
    const auto service_str = std::string("service");
    api::message_header header;
    std::copy(topic_str.begin(), topic_str.end(), header.topic.begin());
    std::copy(service_str.begin(), service_str.end(), header.service_name.begin());

    const auto subscribe_to_topic = std::string("aaa");
    api::subscribe_message subscribe_message{};
    std::copy(subscribe_to_topic.begin(), subscribe_to_topic.end(), subscribe_message.topic_to_subscribe_to.begin());

    std::vector<unsigned char> body_bytes(sizeof(api::subscribe_message));
    memcpy(&body_bytes[0], &subscribe_message, sizeof(api::subscribe_message));

    client.send_message(header, body_bytes);

    std::thread t([&]()
        { keep_running(); });
    t.join();
}
