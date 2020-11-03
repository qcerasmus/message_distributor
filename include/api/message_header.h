#pragma once

#include <array>
#include <iostream>

#include <cereal/types/array.hpp>

#pragma pack(1)
namespace api
{
    struct message_header
    {
        std::array<char, 25> topic{};
        std::array<char, 25> service_name{};
        std::uint64_t body_length = 0;

        std::string get_topic_string()
        {
            auto topic_str = std::string(topic.begin(), topic.end());
            topic_str.erase(std::find(topic_str.begin(), topic_str.end(), '\0'), topic_str.end());
            return topic_str;
        }

        std::string get_service_name_string()
        {
            auto service_name_str = std::string(service_name.begin(), service_name.end());
            service_name_str.erase(std::find(service_name_str.begin(), service_name_str.end(), '\0'), service_name_str.end());
            return service_name_str;
        }

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(topic, service_name, body_length);
        }
    };
}
#pragma pack()
