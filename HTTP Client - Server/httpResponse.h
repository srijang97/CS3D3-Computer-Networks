#ifndef httpResponse_H
#define httpResponse_H

#include <string>
#include <iostream>

class httpResponse
{
    std::string httpVer;
    unsigned int status_code;
    std::string phrase;
    std::string messageBody;
    
    std::string toString(int);
    
    void buildResponse();

    public:
        httpResponse(unsigned int, std::string);
        int getResponseLength(); 
        std::string fullResponse;
    };
    
    #endif
