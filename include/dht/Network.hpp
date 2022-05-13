#ifndef NETWORK_HPP
#define NETWORK_HPP

#include "Node.hpp"
#include "Message.hpp"

class Network{
    public:
    // Existing RPC methods
    virtual bool sendPing(const Node& node);  // Returns success/failure
    virtual std::vector<Node> sendFindNode(const Node& node, const std::string& target_id);
    virtual bool sendStore(const Node& node, const std::string& key, const std::string& value);
    virtual std::optional<std::string> sendFindValue(const Node& node, const std::string& key);

    // Response handlers
    virtual void handlePingResponse(const Node& from);
    virtual void handleFindNodeResponse(const Node& from, const std::vector<Node>& nodes);
    virtual void handleStoreResponse(const Node& from, bool success);
    virtual void handleFindValueResponse(const Node& from, const std::string& key, const std::optional<std::string>& value);

    // Connection management
    virtual bool connect(const Node& node);
    virtual void disconnect(const Node& node);
    virtual bool isConnected(const Node& node) const;

    // Message timeouts and retries
    void setRequestTimeout(std::chrono::milliseconds timeout);
    void setMaxRetries(unsigned int retries);

    // Network statistics
    size_t getActiveConnections() const;
    double getAverageLatency() const;
    
    virtual ~Network() = default;

    protected:
    // Helper methods for implementing concrete network layers
    virtual void sendMessage(const Node& node, const Message& message) = 0;
    virtual void handleResponse(const Node& from, const Message& response) = 0;

    private:
    std::chrono::milliseconds request_timeout{1000}; // Default 1 second
    unsigned int max_retries{3};
};


#endif