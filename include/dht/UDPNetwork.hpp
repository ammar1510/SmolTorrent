#ifndef UDP_NETWORK_HPP
#define UDP_NETWORK_HPP

#include <mutex>
#include <unordered_map>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <random>
#include <sstream>
#include <iomanip>

#include "Network.hpp"
#include "Message.hpp"


class UDPNetwork : public Network{
    public:
    UDPNetwork(uint16_t port, RoutingTable& routingTable, std::map<std::string, std::string>& dataStore);
    ~UDPNetwork();
    bool sendPing(const Node& node) override{
        try{
            Message ping_msg;
            ping_msg.type = MessageType::PING;
            ping_msg.message_id = generateMessageId();
            sendMessage(node, ping_msg);
            if(waitForResponse(ping_msg.message_id,request_timeout)){
                std::lock_guard<std::mutex> lock(connections_mutex);
                active_connections[node.getId()] = true;
                return true;
            }
        }
        catch(const std::exception& e){
            return false;
        }
    }
    std::vector<Node> sendFindNode(const Node& node, const std::string& target_id) override {
        Message find_node_msg;
        find_node_msg.type = MessageType::FIND_NODE;
        find_node_msg.message_id = generateMessageId();
        find_node_msg.payload["target_id"] = target_id;
        sendMessage(node, find_node_msg);

        if (waitForResponse(find_node_msg.message_id, request_timeout)) {
            std::lock_guard<std::mutex> lock(connections_mutex);
            return received_nodes[find_node_msg.message_id];
        } else {
            return {};
        }
    }
    bool sendStore(const Node& node, const std::string& key, const std::string& value) override {
        Message store_msg;
        store_msg.type = MessageType::STORE;
        store_msg.message_id = generateMessageId();
        store_msg.payload["key"] = key;
        store_msg.payload["value"] = value;
        sendMessage(node, store_msg);

        return waitForResponse(store_msg.message_id, request_timeout);
    }
    std::optional<std::string> sendFindValue(const Node& node, const std::string& key) override {
        Message find_value_msg;
        find_value_msg.type = MessageType::FIND_VALUE;
        find_value_msg.message_id = generateMessageId();
        find_value_msg.payload["key"] = key;
        sendMessage(node, find_value_msg);

        if (waitForResponse(find_value_msg.message_id, request_timeout)) {
            std::lock_guard<std::mutex> lock(connections_mutex);
            if (received_values.count(find_value_msg.message_id) > 0) {
                return received_values[find_value_msg.message_id];
            } else {
                return std::nullopt;
            }
        } else {
            return std::nullopt;
        }
    }
    void sendMessage(const Node& node, const Message& message) override{
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(node.getPort());
        addr.sin_addr.s_addr = inet_addr(node.getAddress().c_str());
        std::string serialized = message.serialize();
        sendto(socket_fd, serialized.c_str(), serialized.size(), 0, (struct sockaddr*)&addr, sizeof(addr));
    }
    void handleResponse(const Node& from, const Message& response,
                        RoutingTable& routingTable,
                        std::map<std::string, std::string>& dataStore) {
        std::lock_guard<std::mutex> lock(connections_mutex);
        if (response.type == MessageType::PING_RESPONSE ||
            response.type == MessageType::STORE_RESPONSE) {
            responses[response.message_id] = true;
        } else if (response.type == MessageType::FIND_NODE_RESPONSE) {
            if (response.payload.count("nodes") > 0) {
                // Assuming nodes are sent as a comma-separated string of "ip:port" pairs
                std::string nodes_str = response.payload.at("nodes");
                std::vector<Node> nodes;
                std::stringstream ss(nodes_str);
                std::string node_str;
                while (std::getline(ss, node_str, ',')) {
                    std::stringstream node_ss(node_str);
                    std::string ip, port_str;
                    std::getline(node_ss, ip, ':');
                    std::getline(node_ss, port_str);
                    nodes.emplace_back(ip, std::stoi(port_str));
                }
                received_nodes[response.message_id] = nodes;
            }
        } else if (response.type == MessageType::FIND_VALUE_RESPONSE) {
            if (response.payload.count("value") > 0) {
                received_values[response.message_id] = response.payload.at("value");
            }
        } else if (response.type == MessageType::PING) {
            Message response;
            response.type = MessageType::PING_RESPONSE;
            response.message_id = generateMessageId();
            sendMessage(from, response);
        } else if (response.type == MessageType::FIND_NODE) {
            std::string target_id = response.payload.at("target_id");
            std::vector<Node> closest_nodes = routingTable.findClosestNodes(target_id);

            std::string nodes_str;
            for (const Node& node : closest_nodes) {
                nodes_str += node.getAddress() + ",";
            }
            if (!nodes_str.empty()) {
                nodes_str.pop_back(); // Remove trailing comma
            }

            Message response;
            response.type = MessageType::FIND_NODE_RESPONSE;
            response.message_id = generateMessageId();
            response.payload["nodes"] = nodes_str;
            sendMessage(from, response);
        } else if (response.type == MessageType::STORE) {
            std::string key = response.payload.at("key");
            std::string value = response.payload.at("value");
            dataStore[key] = value; // Store the key-value pair

            Message response;
            response.type = MessageType::STORE_RESPONSE;
            response.message_id = generateMessageId();
            sendMessage(from, response);
        } else if (response.type == MessageType::FIND_VALUE) {
            std::string key = response.payload.at("key");
            if (dataStore.count(key) > 0) {
                Message response;
                response.type = MessageType::FIND_VALUE_RESPONSE;
                response.message_id = generateMessageId();
                response.payload["value"] = dataStore[key];
                sendMessage(from, response);
            } else {
                // If value not found, treat as FIND_NODE
                std::string target_id = key;
                std::vector<Node> closest_nodes = routingTable.findClosestNodes(target_id);

                std::string nodes_str;
                for (const Node& node : closest_nodes) {
                    nodes_str += node.getAddress() + ",";
                }
                if (!nodes_str.empty()) {
                    nodes_str.pop_back(); // Remove trailing comma
                }

                Message response;
                response.type = MessageType::FIND_NODE_RESPONSE;
                response.message_id = generateMessageId();
                response.payload["nodes"] = nodes_str;
                sendMessage(from, response);
            }
        }
    }
    private:
        int socket_fd;
        std::chrono::milliseconds request_timeout = std::chrono::milliseconds(5000);
        uint16_t port;
        std::unordered_map<std::string,bool>active_connections;
        std::mutex connections_mutex;
        std::unordered_map<std::string, bool> responses;
        std::unordered_map<std::string, std::vector<Node>> received_nodes;
        std::unordered_map<std::string, std::string> received_values;
        RoutingTable& routingTable;
        std::map<std::string, std::string>& dataStore;
        bool initSocket(uint16_t port){
            socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
            if(socket_fd == -1){
                return false;
            }
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = INADDR_ANY;
            if(bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1){
                return false;
            }
            std::thread(&UDPNetwork::listenForResponses, this, std::ref(routingTable), std::ref(dataStore)).detach();
            return true;
        }
        void listenForResponses(RoutingTable& routingTable, std::map<std::string, std::string>& dataStore) {
            while (true) {
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                char buffer[4096] = {0};

                ssize_t bytes_received = recvfrom(socket_fd, buffer, sizeof(buffer), 0,
                                                  (struct sockaddr*)&client_addr, &client_addr_len);

                if (bytes_received > 0) {
                    std::string received_data(buffer, bytes_received);
                    Message response = Message::deserialize(received_data);
                    std::string ip_address = inet_ntoa(client_addr.sin_addr);
                    std::string address = ip_address + ":" + std::to_string(ntohs(client_addr.sin_port));
                    Node from(ip_address, ntohs(client_addr.sin_port));
                    handleResponse(from, response, routingTable, dataStore);
                }
            }
        }
        bool waitForResponse(const std::string& message_id, std::chrono::milliseconds timeout) {
            auto start_time = std::chrono::steady_clock::now();
            while (std::chrono::steady_clock::now() - start_time < timeout) {
                {
                    std::lock_guard<std::mutex> lock(connections_mutex);
                    if (responses.count(message_id) > 0) {
                        responses.erase(message_id);
                        return true;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            return false;
        }
        std::string generateMessageId() {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_int_distribution<> distrib(0, 255);

            std::stringstream ss;
            for (int i = 0; i < 16; ++i) {
                ss << std::hex << distrib(gen);
            }
            return ss.str();
        }
};




#endif