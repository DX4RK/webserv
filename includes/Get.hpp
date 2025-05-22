#pragma once

#include <iostream>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "utils.hpp"
#include "method.hpp"
#include "request.hpp"

class Response;
class Request;

class Get : public Method {
public:
	Get( Request &rq );
	~Get( void );

	void process( Response &response, Request &request );
private:
	Get( void );

	int _fileFd;
};
