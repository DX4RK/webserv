#pragma once

#include "_libs.hpp"

#include "config.hpp"

class CGI {
public:
	CGI( std::string method, std::string protocol, std::map<std::string, std::string> headers );
	~CGI( void );

	void execute( void );
	char **formatEnvironment( void );
	void setEnvironment( std::string scriptPath, Config &config );
private:
	void _addEnv(std::string index, std::string value);
	bool _addEnvHeader(std::string index, std::string headerIndex);

	char **_envp;
	bool _envpFormatted;

	std::string _method;
	std::string _protocol;
	std::map<std::string, std::string> _headers;

	std::map<std::string, std::string> _env;

	std::string _findHeader( std::string index );
};
