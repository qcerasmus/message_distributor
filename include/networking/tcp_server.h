#pragma once

#include <sstream>

#include <asio.hpp>
#include <asio/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <concurrentqueue.h>

#include "connection.h"

namespace networking
{
    class tcp_server
    {
    public:
        explicit tcp_server(const std::string& ip_address, const short port);
        ~tcp_server();

        bool start();
        bool stop();

    protected:
        void wait_for_client();
        void message_to_forward();

    private:
        std::string _ip_address;
        short _port;

        asio::io_context _io_context;
        asio::ip::tcp::endpoint _endpoint;
        std::unique_ptr<asio::ip::tcp::acceptor> _acceptor;
        std::thread _context_thread;
        std::vector<std::shared_ptr<connection>> _connections;

        std::shared_ptr<moodycamel::ConcurrentQueue<networking::message_packet>> _message_queue;
    };

    inline tcp_server::tcp_server(const std::string& ip_address, const short port)
        : _ip_address(ip_address),
        _port(port)
    {
        try
        {
            _message_queue = std::make_shared<moodycamel::ConcurrentQueue<message_packet>>(1'000'000);
            _endpoint = asio::ip::tcp::endpoint(asio::ip::make_address(_ip_address), port);
            _acceptor = std::make_unique<asio::ip::tcp::acceptor>(_io_context, _endpoint);
        }
        catch(const std::exception& e)
        {
            spdlog::error("[tcp_server] Error: {}", e.what());
        }
    }

    inline tcp_server::~tcp_server()
    {
        _connections.clear();
        stop();
    }

    inline bool tcp_server::start()
    {
        try
        {
            wait_for_client();
            _context_thread = std::thread([this]()
                {
                    _io_context.run();
                });
        }
        catch(const std::exception& e)
        {
            spdlog::error("[tcp_server] Error: {}", e.what());
            return false;
        }

        spdlog::info("[tcp_server] Started on {}:{}", _ip_address, _port);
        return true;
    }

    inline bool tcp_server::stop()
    {
        _io_context.stop();
        if (_context_thread.joinable())
            _context_thread.join();

        spdlog::info("[tcp_server] Stopped...");
        return true;
    }

    inline void tcp_server::wait_for_client()
    {
        _acceptor->async_accept([this](std::error_code ec, asio::ip::tcp::socket socket)
            {
                if (!ec)
                {
                    std::stringstream str;
                    str << socket.remote_endpoint();
                    spdlog::debug("[tcp_server] a new connection: {}", str.str());
                    auto new_connection = std::make_shared<connection>(_io_context, std::move(socket),
                        _message_queue);
                    new_connection->message_received = [&]()
                    {
                        message_to_forward();
                    };

                    _connections.emplace_back(new_connection);
                    wait_for_client();
                }
                else
                {
                    spdlog::error("[tcp_server] Error: ", ec.message());
                }
            });
    }

    inline void tcp_server::message_to_forward()
    {
        message_packet mp{};
        _message_queue->try_dequeue(mp);
        for(const auto& connection : _connections)
        {
            if (connection->my_endpoint != mp.endpoint_)
            {
                spdlog::debug("sending packet to endpoint: {}", connection->my_endpoint);
            }
        }
    }

}