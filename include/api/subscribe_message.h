#pragma once

#pragma pack(1)
namespace api
{
    struct subscribe_message
    {
        std::array<char, 25> topic_to_subscribe_to{};

        std::string get_topic_to_subscribe_to_string() const
        {
            auto topic_str = std::string(topic_to_subscribe_to.begin(), topic_to_subscribe_to.end());
            topic_str.erase(std::find(topic_str.begin(), topic_str.end(), '\0'), topic_str.end());
            return topic_str;
        }
    };
}
#pragma pack()
