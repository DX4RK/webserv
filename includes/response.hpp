#pragma once

#include <iostream>
#include "request.hpp"

class Request;

class Response {
public:
	Response( Request &request );
private:
	Response( void );

	std::string _response;
};
