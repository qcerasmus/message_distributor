#include <iostream>

#include <spdlog/spdlog.h>

#include <asio.hpp>
#include <asio/buffer.hpp>
#include <asio/ts/internet.hpp>

#include "api/message_header.h"

void keep_running()
{
    std::getchar();
}

int main()
{
    asio::io_context io_context;
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address("127.0.0.1"), 9876);
    asio::ip::tcp::socket socket(io_context);

    socket.connect(endpoint);

    api::message_header mh;
    mh.message_length = 15;
    mh.topic = {'A'};
    mh.service_name = { 'S' };
    unsigned char header_bytes[sizeof(api::message_header) + 15];
    std::vector<unsigned char> body = { 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A' };
    std::memcpy(&header_bytes[0], &mh, sizeof(api::message_header));
    std::memcpy(&header_bytes[sizeof(api::message_header)], body.data(), 15);


    asio::async_write(socket, asio::buffer(header_bytes, sizeof(header_bytes)),
        [&](std::error_code ec, std::size_t length_written)
        {
            if (!ec)
            {
                if (length_written == sizeof(api::message_header))
                {
                    spdlog::debug("[client] we wrote a message header");
                }
                else
                {
                    spdlog::error("[client] we didn't write a full message header");
                }
            }
            else
            {
                spdlog::error("[client] Error: {}", ec.message());
            }
        });

    asio::async_read(socket, asio::buffer(header_bytes, sizeof(header_bytes)),
        [&](std::error_code ec, std::size_t length_read)
        {
            if (!ec)
            {
                if (length_read == sizeof(header_bytes))
                {
                    spdlog::debug("We received a packet");
                }
            }
            else
            {
                //error
            }
        });

    std::thread t([&]()
        {
            keep_running();
        });
    t.join();
}