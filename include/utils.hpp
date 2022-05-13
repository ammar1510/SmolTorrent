#ifndef UTILS_HPP
#define UTILS_HPP
#include <string>
#include <bitset>
#include "sha256.h"

std::string hexToBinary(std::string hex){
    std::string binary = "";
    for(size_t i = 0; i < hex.size(); i++){
        int val = (hex[i] >= '0' && hex[i] <= '9') ? hex[i] - '0' : hex[i] - 'a' + 10;
        binary += std::bitset<4>(val).to_string();
    }
    return binary;
}

std::string binaryToHex(std::string binary){
    std::string hex = "";
    for(size_t i = 0; i < binary.size(); i+=4){
        int val = std::bitset<4>(binary.substr(i, 4)).to_ulong();
        if(val < 10){
            hex += '0' + val;
        }else{
            hex += 'a' + val - 10;
        }   
    }
    return hex;
}

std::string xorDistance(std::string hex1, std::string hex2){
    std::string bin1 = hexToBinary(hex1);
    std::string bin2 = hexToBinary(hex2);
    std::string result = "";
    for(size_t i = 0; i < bin1.size(); i++){
        result.push_back((bin1[i]!= bin2[i])+'0');
    }
    return binaryToHex(result);
}


std::string getSha256(std::string data){
    SHA256 sha;
    sha.update(data);
    auto hash = sha.digest();
    return SHA256::toString(hash);
}
#endif
