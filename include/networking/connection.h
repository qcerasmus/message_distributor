#pragma once

#include <memory>

#include <asio.hpp>
#include <concurrentqueue.h>
#include <utility>
#include <spdlog/spdlog.h>

#include "api/message_header.h"
#include "networking/message_packet.h"

namespace networking
{
class connection : public std::enable_shared_from_this<connection>
{
  public:
    connection(asio::io_context &io_context, asio::ip::tcp::socket socket,
        std::shared_ptr<moodycamel::ConcurrentQueue<message_packet>> message_queue);
    connection(const connection &c) = delete;
    connection(const connection &&c) = delete;
    connection &operator=(const connection &other) = delete;
    connection &operator=(const connection &&other) = delete;
    ~connection();

    std::function<void()> message_received;
    std::function<void(const std::string &endpoint)> client_disconnected;

    void disconnect();
    void send_message(const message_packet &message_to_send);

    std::string my_endpoint;
    std::string service_name;

  protected:
    asio::io_context &_io_context;
    asio::ip::tcp::socket _socket;

  private:
    void send_message();
    void write_body();
    void read_header();
    void read_body();
    std::shared_ptr<moodycamel::ConcurrentQueue<message_packet>> _message_queue;

    message_packet _packet;
    message_packet _current_packet_to_write;
    moodycamel::ConcurrentQueue<message_packet> _packets_to_write;
    bool _done_writing = true;
};

inline connection::connection(asio::io_context &io_context, asio::ip::tcp::socket socket,
    std::shared_ptr<moodycamel::ConcurrentQueue<message_packet>> message_queue)
    : _io_context(io_context),
      _socket(std::move(socket)),
      _message_queue(std::move(message_queue))
{
    std::stringstream str;
    str << _socket.remote_endpoint();
    my_endpoint = str.str();
    _socket.set_option(asio::ip::tcp::no_delay(true));

    read_header();
}

inline connection::~connection()
{
    if (_socket.is_open())
        _socket.close();
}

inline void connection::disconnect()
{
    _socket.close();
}

inline void connection::send_message(const message_packet &message_to_send)
{
    _packets_to_write.enqueue(message_to_send);
    send_message();
}

inline void connection::send_message()
{
    // we're currently writing a packet, just chill for a bit
    while (!_done_writing)
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(100));
    }

    if (_packets_to_write.try_dequeue(_current_packet_to_write))
    {
        _done_writing = false;
        asio::async_write(_socket, asio::buffer(&_current_packet_to_write.header_, sizeof(api::message_header)),
            [&](std::error_code ec, std::size_t length_written)
            {
                if (!ec)
                {
                    if (length_written == sizeof(api::message_header))
                    {
                        spdlog::trace("[connection] sent header!");
                        write_body();
                    }
                    else
                    {
                        spdlog::trace("[connection] we sent less bytes than expected for the header");
                    }
                }
            });
    }
}

inline void connection::write_body()
{
    asio::post(_io_context, [&]()
        { asio::async_write(_socket, asio::dynamic_buffer(_current_packet_to_write.body_, _current_packet_to_write.header_.body_length),
              [&](std::error_code ec, std::size_t length_written)
              {
                  if (!ec)
                  {
                      if (length_written == _current_packet_to_write.header_.body_length)
                      {
                          spdlog::trace("[connection] sent body!");
                          _done_writing = true;
                      }
                      else
                      {
                          spdlog::trace("[connection] we sent less bytes than expected for the body");
                      }
                  }
              }); });
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
                    if (service_name.empty())
                        service_name = _packet.header_.get_service_name_string();

                    _packet.endpoint_ = my_endpoint;
                    spdlog::trace("[connection] The length of the body is: {}", _packet.header_.body_length);

                    read_body();
                }
                else
                {
                    spdlog::error("[connection] we received less bytes in the header than expected...");
                }
            }
            else if (ec.value() == 2 || ec.value() == 10054) // Connection was closed. Client disconnected
            {
                return;
            }
            else
            {
                spdlog::error("[connection] Error: {}, Error value: {}", ec.message(), ec.value());
                return;
            }

            read_header();
        });
}

inline void connection::read_body()
{
    asio::async_read(_socket, asio::dynamic_buffer(_packet.body_, _packet.header_.body_length),
        [&](std::error_code ec, std::size_t length_read)
        {
            if (!ec)
            {
                if (length_read == _packet.header_.body_length)
                {
                    spdlog::trace("[connection] We have received an entire packet with topic: {}", _packet.header_.get_topic_string());
                    _message_queue->enqueue(_packet);
                    _packet.body_.clear();
                    if (message_received)
                        message_received();
                }
                else
                {
                    spdlog::error("[connection] we received: {} bytes but expected: {} bytes", length_read, _packet.header_.body_length);
                }
            }
            else if (ec.value() == 2) // Connection was closed. Client disconnected
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
} // namespace networking
