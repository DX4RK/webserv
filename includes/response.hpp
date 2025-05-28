#pragma once

#include <iostream>

#include "Get.hpp"
#include "Post.hpp"
#include "utils.hpp"
#include "method.hpp"
#include "request.hpp"

class Request;

class Response {
public:
	Response( Request &request, Config &server_config );
	~Response( void );

	int getResponseCode(void) const;
	std::string getResponse( void ) const;

	void addHeader( std::string headerName, std::string headerValue );
private:
	int _responseCode;

	std::string _response;
	std::string _headers;

	Method _processRequest(std::string method, Request &request, Config &server_config);

	Response( void );
};
