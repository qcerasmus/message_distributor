#pragma once

#include <string>
#include <api/message_header.h>

#include <cereal/types/array.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

namespace networking
{
    struct message_packet
    {
        std::string endpoint_;
        api::message_header header_;
        std::vector<unsigned char> body_;

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(endpoint_, header_, body_);
        }
    };
}
