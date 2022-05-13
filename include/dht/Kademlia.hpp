#ifndef KADEMLIA_HPP
#define KADEMLIA_HPP

#include <algorithm>
#include <iostream>
#include <map>
#include <stdexcept>
#include <vector>
#include <thread>

#include "Network.hpp"
#include "Node.hpp"
#include "RoutingTable.hpp"
#include "UDPNetwork.hpp"

class DHT {
 private:
  Node localNode;
  RoutingTable routingTable;
  std::unique_ptr<Network> networkLayer;
  std::map<std::string, std::string> dataStore;

  static constexpr int k = 20;
  static constexpr int alpha = 3;

 public:
  DHT(uint16_t port)
      : localNode("", port),
        routingTable(localNode),
        networkLayer(std::make_unique<UDPNetwork>(port, routingTable, dataStore)) {}

  void joinNetwork(const std::vector<std::string>& bootstrapNodes) {
    for (const std::string& node : bootstrapNodes) {
      try {
        Node n(node);
        routingTable.addNode(n);
      } catch (const std::exception& e) {
        std::cerr << "Error adding bootstrap node: " << e.what() << std::endl;
      }
    }

    // Perform iterative lookup on own ID to discover more nodes
    iterativeFindNode(localNode.getId());

    // Store the local node's contact info in the DHT
    storeValue(localNode.getId(), localNode.getAddress());
  }

  void run() {
    std::thread(&UDPNetwork::listenForResponses, networkLayer.get(), std::ref(routingTable), std::ref(dataStore)).detach();

    // TODO: Implement peer connection and data exchange logic here
    // Periodically refresh the routing table
    while (true) {
      refreshRoutingTable();
      std::this_thread::sleep_for(std::chrono::minutes(5));
    }
  }

  void refreshRoutingTable() {
    // Iterate through all buckets in the routing table
    for (int i = 0; i < 160; ++i) {
      std::vector<Node> nodes = routingTable.getNodesInBucket(i);
      for (const Node& node : nodes) {
        // Check if the node is still active
        if (!networkLayer->sendPing(node)) {
          // If not active, remove it from the routing table
          routingTable.removeNode(node);
        }
      }
    }
  }

  std::optional<std::string> findValue(const std::string& key) {
    // First, check the local data store
    if (dataStore.count(key) > 0) {
      return dataStore[key];
    }

    // If not found locally, perform an iterative search
    std::vector<Node> closestNodes = iterativeFindNode(key);
    for (const Node& node : closestNodes) {
      std::optional<std::string> value =
          networkLayer->sendFindValue(node, key);
      if (value.has_value()) {
        return value;
      }
    }
    return std::nullopt;
  }

  bool storeValue(const std::string& key, const std::string& value) {
    std::vector<Node> closestNodes = iterativeFindNode(key);
    bool storedSuccessfully = false;

    for (const Node& node : closestNodes) {
      if (networkLayer->sendStore(node, key, value)) {
        storedSuccessfully = true;
      }
    }

    // Additionally, store the value locally
    dataStore[key] = value;

    return storedSuccessfully;
  }

  void bootstrap(const std::string& ip, uint16_t port) {
    try {
      Node n(ip, port);
      routingTable.addNode(n);
    } catch (const std::exception& e) {
      std::cerr << "Error bootstrapping to " << ip << ":" << port << ": "
                << e.what() << std::endl;
    }
  }

 private:
  std::vector<Node> iterativeFindNode(const std::string& targetId) {
    std::vector<Node> closestNodes = routingTable.findClosestNodes(targetId);
    std::vector<Node> queriedNodes;
    std::vector<Node> newNodes;

    while (true) {
      int queryCount = 0;
      for (const Node& node : closestNodes) {
        if (std::find(queriedNodes.begin(), queriedNodes.end(), node) ==
                queriedNodes.end() &&
            queryCount < alpha) {
          std::vector<Node> foundNodes =
              networkLayer->sendFindNode(node, targetId);
          newNodes.insert(newNodes.end(), foundNodes.begin(), foundNodes.end());
          queriedNodes.push_back(node);
          queryCount++;
        }
      }

      if (queryCount == 0) {
        break;
      }

      std::sort(newNodes.begin(), newNodes.end(),
                [&targetId](const Node& a, const Node& b) {
                  return xorDistance(a.getId(), targetId) <
                         xorDistance(b.getId(), targetId);
                });

      closestNodes.insert(closestNodes.end(), newNodes.begin(), newNodes.end());
      std::sort(closestNodes.begin(), closestNodes.end(),
                [&targetId](const Node& a, const Node& b) {
                  return xorDistance(a.getId(), targetId) <
                         xorDistance(b.getId(), targetId);
                });

      auto last = std::unique(closestNodes.begin(), closestNodes.end());
      closestNodes.erase(last, closestNodes.end());

      if (closestNodes.size() > k) {
        closestNodes.resize(k);
      }

      newNodes.clear();
    }

    return closestNodes;
  }
};

#endif  // KADEMLIA_HPP




