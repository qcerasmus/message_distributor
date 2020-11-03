#pragma once

#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <fstream>

#include <asio.hpp>
#include <asio/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <concurrentqueue.h>
#include <cereal/archives/binary.hpp>

#include "connection.h"
#include "api/subscribe_message.h"

namespace networking
{
    class tcp_server
    {
    public:
        explicit tcp_server(const std::string& ip_address, const unsigned short port);
        ~tcp_server();

        bool start();
        bool stop();

    protected:
        void wait_for_client();
        void message_to_forward();

    private:
        std::string _ip_address;
        unsigned short _port;

        asio::io_context _io_context;
        asio::ip::tcp::endpoint _endpoint;
        std::unique_ptr<asio::ip::tcp::acceptor> _acceptor;
        std::thread _context_thread;
        std::map<std::unique_ptr<connection>, std::vector<std::string>> _connections;

        std::shared_ptr<moodycamel::ConcurrentQueue<networking::message_packet>> _message_queue;
    };

    inline tcp_server::tcp_server(const std::string& ip_address, const unsigned short port)
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
        for(const auto& [connection, topics]: _connections)
        {
            connection->disconnect();
        }
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
                    auto new_connection = std::make_unique<connection>(_io_context, std::move(socket), _message_queue);
                    new_connection->message_received = [&]()
                    {
                        message_to_forward();
                    };

                    _connections[(std::move(new_connection))];
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
        while (_message_queue->try_dequeue(mp))
        {
            //Forward messages to other distributors...


            auto topic_string = mp.header_.get_topic_string();
            if (topic_string == "sub")
            {
                const auto* subsciption_message = reinterpret_cast<api::subscribe_message*>(&mp.body_[0]);

                for (auto& [connection, topics] : _connections)
                {
                    if (connection->my_endpoint == mp.endpoint_)
                    {
                        spdlog::debug("[tcp_server] client endpoint: {} subscribed to topic: {}", connection->my_endpoint, subsciption_message->get_topic_to_subscribe_to_string());
                        topics.push_back(subsciption_message->get_topic_to_subscribe_to_string());

                        return;
                    }
                }

                return;
            }

            {
                std::ofstream os("messages.cereal", std::ios::binary);
                cereal::BinaryOutputArchive archive(os);

                archive(mp);
            }

            for (const auto& [connection, topics] : _connections)
            {
                if (connection->my_endpoint != mp.endpoint_ || connection->service_name == mp.header_.get_service_name_string())
                {
                    for (const auto& topic : topics)
                    {
                        if (topic == topic_string)
                        {
                            spdlog::info("[tcp_server] sending packet to endpoint: {}", connection->my_endpoint);
                            connection->send_message(mp);
                            break;
                        }
                    }
                }
            }
        }
    }

}
