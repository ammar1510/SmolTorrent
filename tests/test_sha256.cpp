#include <iostream>
#include <string>
#include "../include/sha256.h"

int main() {
    // Example test vector
    std::string input = "abc";

    // Instantiate SHA256 object
    SHA256 sha;

    // Update the hash with input data
    sha.update(input);

    // Calculate the final hash
    auto hash = sha.digest();

    // Convert the hash to a string representation (depending on how it's implemented)
    std::string result = sha.toString(hash);

    // Output the result (hash in hexadecimal form)
    std::cout << result.size()<< std::endl;
    std::cout << "Hash: " << result << std::endl;

    return 0;
}
