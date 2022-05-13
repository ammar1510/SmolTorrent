#ifndef NODE_HPP    
#define NODE_HPP

#include <string>
#include "utils.hpp"

class Node{
    public:
    Node(const std::string& ip_address, uint16_t port)
        : ip_address(ip_address), port(port) {
        this->id = generateId(ip_address, port);
    }

    Node(const std::string& id, const std::string& ip_address, uint16_t port)
        : id(id), ip_address(ip_address), port(port) {}

    std::string getDistance(const Node& other) const{
        return xorDistance(this->id, other.getId());
    }

    std::string getId() const{return this->id;};
    std::string getAddress() const{return this->ip_address;};
    uint16_t getPort() const{return this->port;};
    bool operator==(const Node& other) const{return this->id == other.id;};
    void updateLastSeen(){this->last_seen = std::time(nullptr);};
    time_t getLastSeen() const{return this->last_seen;};
    bool isAlive() const{return std::time(nullptr) - this->last_seen < timeout_threshold;};

    private:
    std::string id,ip_address;
    uint16_t port;
    time_t last_seen = std::time(nullptr);
    static const time_t timeout_threshold = 900; // 15 minutes in seconds

    static std::string generateId(const std::string& ip_address, uint16_t port) {
        return getSha256(ip_address + ":" + std::to_string(port));
    }
};

#endif
