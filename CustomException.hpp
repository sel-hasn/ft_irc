
#ifndef CUSTOMEXCEPTION_HPP
#define CUSTOMEXCEPTION_HPP

#include <iostream>

class CustomException
{
    CustomException();
    CustomException& operator=(const CustomException& other);
    
    std::string Message;
    public:
        CustomException(const CustomException& other);
        ~CustomException();
        CustomException(std::string error);
        const char* msg() const;
};


#endif