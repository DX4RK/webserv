#pragma once

#include <iostream>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "method.hpp"
#include "request.hpp"

class Request;

class Get : public Method {
public:
	Get( Request &rq );
	~Get( void );

	void process( Request &rq );
private:
	Get( void );

	int _fileFd;
};
