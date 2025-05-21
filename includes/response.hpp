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
private:
	Response( void );

	std::string _response;
	void _processRequest(std::string method, Request &request);
};
