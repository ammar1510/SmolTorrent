#include <iostream>
#include <../include/utils.hpp>


int main(){
    std::string hex = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
    std::string bin = hexToBinary(hex);
    std::cout<<bin<<std::endl;
    std::string hex2 = binaryToHex(bin);
    std::cout<<hex2.size()<<std::endl;
    if(hex2 == hex){
        std::cout<<"Success"<<std::endl;
    }else{
        std::cout<<"Failed"<<std::endl;
    }   
    return 0;   
}