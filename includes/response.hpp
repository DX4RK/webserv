#pragma once

#include <iostream>

#include "Get.hpp"
#include "utils.hpp"
#include "method.hpp"
#include "request.hpp"

class Request;

class Response {
public:
	Response( Request &request );
	~Response( void );

	std::string getResponse( void );

	void addHeader( std::string headerName, std::string headerValue );
private:
	Response( void );

	std::string _response;
	std::string _headers;
	Method _processRequest(std::string method, Request &request);
};
