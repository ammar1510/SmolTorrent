#include "Kademlia.hpp"

void DHT::run() {
  std::thread(&UDPNetwork::listenForResponses, networkLayer.get(),
              std::ref(routingTable), std::ref(dataStore))
      .detach();

  // TODO: Implement peer connection and data exchange logic here
  // Periodically refresh the routing table
  while (true) {
    refreshRoutingTable();
    std::this_thread::sleep_for(std::chrono::minutes(5));
  }
}

void DHT::refreshRoutingTable() {
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

std::optional<std::string> DHT::findValue(const std::string& key) {
  // Check if the value is present in the local data store
  if (dataStore.count(key) > 0) {
    return dataStore[key];
  }

  // Find the closest nodes to the key
  std::vector<Node> closestNodes = routingTable.findClosestNodes(key);

  // Query the closest nodes for the value
  for (const Node& node : closestNodes) {
    std::optional<std::string> value = networkLayer->sendFindValue(node, key);
    if (value.has_value()) {
      return value;
    }
  }

  return std::nullopt;  // Value not found
} 