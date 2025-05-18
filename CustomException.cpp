
#include "CustomException.hpp"

const char* CustomException::msg() const {
    return Message.c_str();
}

CustomException::~CustomException(){}

CustomException::CustomException(std::string message): Message(message) {}

CustomException::CustomException(const CustomException& other){
    (void) other;
}