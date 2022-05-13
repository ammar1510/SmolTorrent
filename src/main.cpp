#include <iostream>
#include <string>
#include <thread>
#include "CLI11.hpp"
#include "Kademlia.hpp"

int main(int argc, char** argv) {
  CLI::App app{"Kademlia Distributed Hash Table"};

  // Define command line options
  uint16_t port = 0;
  app.add_option("-p,--port", port, "Port to listen on")->required();

  std::string bootstrap_ip;
  uint16_t bootstrap_port = 0;
  auto bootstrap_option =
      app.add_option("-b,--bootstrap", bootstrap_ip,
                     "IP address of bootstrap node");
  app.add_option("-B,--bootstrap-port", bootstrap_port,
                 "Port of bootstrap node");

  // Require bootstrap port if bootstrap IP is provided
  bootstrap_option->needs(
      app.get_option("-B,--bootstrap-port")->required(true));

  CLI11_PARSE(app, argc, argv);

  // Create a DHT node
  DHT dht(port);

  // Bootstrap the node if a bootstrap IP is provided
  if (!bootstrap_ip.empty()) {
    dht.bootstrap(bootstrap_ip, bootstrap_port);
  }

  // Start the DHT node in a separate thread
  std::thread dht_thread(&DHT::run, &dht);

  // Keep the main thread alive for interactive commands
  std::string command;
  while (true) {
    std::cout << "Enter command (find <key>, store <key> <value>, exit): ";
    std::getline(std::cin, command);

    if (command.rfind("find ", 0) == 0) {
      std::string key = command.substr(5);
      std::optional<std::string> value = dht.findValue(key);
      if (value.has_value()) {
        std::cout << "Value: " << value.value() << std::endl;
      } else {
        std::cout << "Value not found." << std::endl;
      }
    } else if (command.rfind("store ", 0) == 0) {
      size_t space_pos = command.find(' ', 6);
      if (space_pos == std::string::npos) {
        std::cerr << "Invalid store command format." << std::endl;
        continue;
      }
      std::string key = command.substr(6, space_pos - 6);
      std::string value = command.substr(space_pos + 1);
      dht.storeValue(key, value);
      std::cout << "Stored key-value pair." << std::endl;
    } else if (command == "exit") {
      break;
    } else {
      std::cerr << "Invalid command." << std::endl;
    }
  }

  // Wait for the DHT thread to finish (it likely won't unless you add a
  // mechanism to stop it)
  dht_thread.join();

  return 0;
}
