#pragma once

#include <memory>

#include <asio.hpp>
#include <concurrentqueue.h>

#include "api/message_header.h"
#include "networking/message_packet.h"

namespace networking
{
    class connection : public std::enable_shared_from_this<connection>
    {
    public:
        connection(asio::io_context& io_context, asio::ip::tcp::socket socket, 
            const std::shared_ptr<moodycamel::ConcurrentQueue<message_packet>>& message_queue);
        connection(const connection& c) = delete;
        connection(const connection&& c) = delete;
        connection& operator=(const connection& other) = delete;
        connection& operator=(const connection&& other) = delete;
        ~connection();

        std::function<void()> message_received;

        std::string my_endpoint;

    protected:
        asio::io_context& _io_context;
        asio::ip::tcp::socket _socket;

    private:
        void read_header();
        void read_body(std::uint64_t length_expected);
        std::shared_ptr<moodycamel::ConcurrentQueue<message_packet>> _message_queue;

        message_packet _packet;
    };

    inline connection::connection(asio::io_context& io_context, asio::ip::tcp::socket socket,
        const std::shared_ptr<moodycamel::ConcurrentQueue<message_packet>>& message_queue)
        : _io_context(io_context),
        _socket(std::move(socket)),
        _message_queue(message_queue)
    {
        std::stringstream str;
        str << _socket.remote_endpoint();
        my_endpoint = str.str();
        _socket.set_option(asio::ip::tcp::no_delay(true));
        read_header();
    }

    inline connection::~connection()
    {
    }

    inline void connection::read_header()
    {
        asio::async_read(_socket, asio::buffer(&_packet.header_, sizeof(api::message_header)),
            [&](std::error_code ec, std::size_t length_read)
            {
                if (!ec)
                {
                    if (length_read == sizeof(api::message_header))
                    {
                        
                        _packet.endpoint_ = my_endpoint;
                        spdlog::debug("[connection] The length of the body is: {}", _packet.header_.message_length);

                        read_body(_packet.header_.message_length);
                    }
                }
                else if (ec.value() == 2 || ec.value() == 10054) //Connection was closed. Client disconnected
                {
                    _socket.close();
                    return;
                }
                else
                {
                    spdlog::error("[connection] Error: {}, Error value: {}", ec.message(), ec.value());
                }

                read_header();
            });
    }

    inline void connection::read_body(std::uint64_t length_expected)
    {
        asio::async_read(_socket, asio::dynamic_buffer(_packet.body_, _packet.header_.message_length),
            [&](std::error_code ec, std::size_t length_read)
            {
                if (!ec)
                {
                    if (length_read == _packet.header_.message_length)
                    {
                        spdlog::debug("[connection] We have received an entire packet");
                        _message_queue->enqueue(_packet);
                        if (message_received)
                            message_received();
                    }
                }
                else if (ec.value() == 2) //Connection was closed. Client disconnected
                {
                    _socket.close();
                    return;
                }
                else
                {
                    spdlog::error("[connection] Error: {}, Error value: {}", ec.message(), ec.value());
                }
            });
    }
}
