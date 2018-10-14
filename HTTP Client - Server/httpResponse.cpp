#include <string>
#include <iostream>
#include <sstream>
#include "httpResponse.h"

httpResponse::httpResponse(unsigned int status_code, std::string messageBody) 
{
    this->httpVer = "HTTP/1.0";
    this->status_code = status_code;
    this->messageBody = messageBody;
    if(status_code == 200)
        this->phrase = "OK";
        else if(status_code == 404)
        this->phrase = "Not found";
        else if(status_code == 400)
        this->phrase = "Bad Request";
    
    this->buildResponse();
}


std::string httpResponse::toString(int value)
{
	std::ostringstream os ;
	os << value ;
    return os.str() ;
}

void httpResponse::buildResponse()
{
    this->fullResponse = this->httpVer + " " + this->toString(this->status_code) + " " + this->phrase + "\r\n\r\n";
    this->fullResponse = this->fullResponse + this->messageBody;
}


int httpResponse::getResponseLength()
{
    return this->fullResponse.length();
}