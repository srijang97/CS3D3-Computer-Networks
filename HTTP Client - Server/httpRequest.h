#ifndef httpRequest_H
#define httpRequest_H

#include <string>
#include <iostream>

class httpRequest 
{
    std::string method;
    std::string httpVer;
    std::string host;
    std::string fullHost;
    std::string uri;
    std::string fullPath;
    
    void setHost(std::string);
    void setUri(std::string);
    void buildRequest();

    public:
        httpRequest(std::string const&, std::string const&, std::string const&);
        int getRequestLength(); 
        std::string fullRequest;
    };
    
    #endif
