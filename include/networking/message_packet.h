#pragma once

#include <string>
#include <api/message_header.h>

namespace networking
{
    class message_packet
    {
    public:
        std::string endpoint_;
        api::message_header header_;
        std::vector<unsigned char> body_;
    };
}
