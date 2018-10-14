#include <string>
#include <iostream>
#include "httpRequest.h"

httpRequest::httpRequest(std::string const& method, std::string const& host, std::string const& path) 
{
    this->method = method;
    this->httpVer = "HTTP/1.0";
    this->setHost(host);
    this->setUri(path);
    this->buildRequest();
}

void httpRequest::setHost(std::string hostName)
{
    this->fullHost = "HOST: " + hostName;
    this->host = hostName;
}

void httpRequest::setUri(std::string uriValue)
{
    this->uri = " " + uriValue + " ";
}

void httpRequest::buildRequest()
{
    this->fullRequest = this->method + this->uri + this->httpVer + "\r\n";
    this->fullRequest = this->fullRequest + this->fullHost + "\r\n"; 
}

int httpRequest::getRequestLength()
{
    return this->fullRequest.length();
}