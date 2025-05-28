#pragma once

#include "request.hpp"
#include "response.hpp"

class CGI {
public:
	~CGI( void );
	CGI( std::string scriptPath, Request *request );
private:
	CGI( void );

	Request *_request;
};
