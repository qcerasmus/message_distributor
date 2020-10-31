#pragma once

#include <array>
#include <iostream>

namespace api
{
    struct message_header
    {
        std::array<char, 25> topic;
        std::array<char, 25> service_name;
        std::uint64_t message_length = 0;

        friend std::ostream& operator<<(std::ostream& os, message_header mh)
        {
            os << "This is the message Header stuff";
            return os;
        }
    };
}
