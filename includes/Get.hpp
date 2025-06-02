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
	Get( Request &request, Config *config );
	~Get( void );

	void process( Response &response, Request &request );
private:
	int _fileFd;

	std::string _fileName;
	std::string _filePath;

	bool _handleFileUrl(Request &request, const std::string root);


	Get( void );
};
