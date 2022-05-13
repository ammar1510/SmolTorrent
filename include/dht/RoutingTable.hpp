#ifndef ROUTING_TABLE_HPP
#define ROUTING_TABLE_HPP

#include <list>
#include <vector>

#include "Node.hpp"

class RoutingTable {
 private:
  Node localNode;
  std::vector<std::list<Node>> buckets;
  static constexpr int k = 20;       // Maximum number of nodes per bucket
  static constexpr int idLength = 160;  // Length of node IDs in bits

 public:
  explicit RoutingTable(const Node& localNode) : localNode(localNode) {
    buckets.resize(idLength);
  }

  void addNode(const Node& node) {
    int bucketIndex = getBucketIndex(node.getId());
    std::list<Node>& bucket = buckets[bucketIndex];

    auto it = std::find(bucket.begin(), bucket.end(), node);
    if (it != bucket.end()) {
      // Node already exists, move it to the end (most recently seen)
      bucket.erase(it);
      bucket.push_back(node);
    } else {
      // New node
      if (bucket.size() < k) {
        bucket.push_back(node);
      } else {
        // Bucket full, ping the least recently seen node
        Node& lruNode = bucket.front();
        // TODO: Implement ping functionality in the network layer
        // If ping fails, replace lruNode with the new node
        // Otherwise, discard the new node
      }
    }
  }

  std::vector<Node> findClosestNodes(const std::string& targetId) const {
    int bucketIndex = getBucketIndex(targetId);
    std::vector<Node> closestNodes;

    // Add nodes from the target bucket
    for (const Node& node : buckets[bucketIndex]) {
      closestNodes.push_back(node);
    }

    // If fewer than k nodes in the target bucket, check adjacent buckets
    for (int i = 1; closestNodes.size() < k && (bucketIndex - i >= 0 || bucketIndex + i < idLength); ++i) {
      if (bucketIndex - i >= 0) {
        for (const Node& node : buckets[bucketIndex - i]) {
          closestNodes.push_back(node);
        }
      }
      if (bucketIndex + i < idLength) {
        for (const Node& node : buckets[bucketIndex + i]) {
          closestNodes.push_back(node);
        }
      }
    }

    // Sort by distance to the target
    std::sort(closestNodes.begin(), closestNodes.end(),
              [&targetId](const Node& a, const Node& b) {
                return xorDistance(a.getId(), targetId) <
                       xorDistance(b.getId(), targetId);
              });

    // Return at most k nodes
    if (closestNodes.size() > k) {
      closestNodes.resize(k);
    }

    return closestNodes;
  }

 private:
  int getBucketIndex(const std::string& nodeId) const {
    // Calculate the XOR distance between the local node ID and the given ID
    std::string distance = xorDistance(localNode.getId(), nodeId);

    // Find the first '1' bit in the distance (most significant bit)
    for (int i = 0; i < idLength; ++i) {
      if (distance[i] == '1') {
        return i;
      }
    }

    return idLength - 1; // Should ideally never reach here
  }
};

#endif  // ROUTING_TABLE_HPP