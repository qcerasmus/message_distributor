#pragma once

#include <spdlog/spdlog.h>

#include <asio.hpp>
#include <asio/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <concurrentqueue.h>

#include "connection.h"

namespace networking
{
    class tcp_client
    {
    public:
        explicit tcp_client(const std::string& ip_address, unsigned short port);
        ~tcp_client();

        std::shared_ptr<moodycamel::ConcurrentQueue<message_packet>> message_queue;
        std::function<void()> message_received;

        void send_message(const api::message_header& header, const std::vector<unsigned char> body);

    private:
        std::unique_ptr<asio::ip::tcp::socket> _socket;
        asio::io_context _io_context;
        std::thread _context_thread;

        std::unique_ptr<connection> _connection;
    };

    inline tcp_client::tcp_client(const std::string& ip_address, unsigned short port)
    {
        message_queue = std::make_shared<moodycamel::ConcurrentQueue<message_packet>>(1'000);
        asio::ip::tcp::endpoint endpoint(asio::ip::make_address(ip_address), port);
        _socket = std::make_unique<asio::ip::tcp::socket>(_io_context);

        _socket->connect(endpoint);

        _connection = std::make_unique<connection>(_io_context, std::move(*_socket), message_queue);
        _connection->message_received = [&]()
        {
            if (message_received)
                message_received();
        };
        _context_thread = std::thread([this]()
            {
                _io_context.run();
            });
    }

    inline tcp_client::~tcp_client()
    {
        _io_context.stop();
        if (_context_thread.joinable())
            _context_thread.join();
    }

    inline void tcp_client::send_message(const api::message_header& header, const std::vector<unsigned char> body)
    {
        networking::message_packet packet;
        packet.endpoint_ = _connection->my_endpoint;
        packet.header_ = header;
        packet.header_.message_length = body.size();
        packet.body_ = body;

        _connection->send_message(packet);
    }
}
