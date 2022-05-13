#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

enum class MessageType {
    PING,
    FIND_NODE,
    STORE,
    FIND_VALUE,
    PING_RESPONSE,
    FIND_NODE_RESPONSE,
    STORE_RESPONSE,
    FIND_VALUE_RESPONSE
};

class Message {
public:
    MessageType type;
    std::string sender_id;  //hash of the sender's address
    std::string message_id;  // For matching responses to requests
    std::map<std::string, std::string> payload;
    
    // Serialization methods
    std::string serialize() const {
        std::stringstream ss;
        ss << static_cast<int>(type) << "|";
        ss << sender_id << "|";
        ss << message_id << "|";

        // Serialize the payload map
        ss << payload.size() << "|";
        for (const auto& pair : payload) {
            ss << pair.first << "|" << pair.second << "|";
        }

        return ss.str();
    }

    static Message deserialize(const std::string& data) {
        Message msg;
        std::stringstream ss(data);
        std::string segment;

        // Deserialize type
        std::getline(ss, segment, '|');
        msg.type = static_cast<MessageType>(std::stoi(segment));

        // Deserialize sender_id
        std::getline(ss, segment, '|');
        msg.sender_id = segment;

        // Deserialize message_id
        std::getline(ss, segment, '|');
        msg.message_id = segment;

        // Deserialize payload size
        std::getline(ss, segment, '|');
        size_t payload_size = std::stoull(segment);

        // Deserialize payload
        for (size_t i = 0; i < payload_size; ++i) {
            std::string key, value;
            std::getline(ss, key, '|');
            std::getline(ss, value, '|');
            msg.payload[key] = value;
        }

        return msg;
    }
};

#endif