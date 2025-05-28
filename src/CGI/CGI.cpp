#include "CGI.hpp"

CGI::CGI(void) {}
CGI::~CGI(void) {}
CGI::CGI(std::string scriptPath, Request *request) : _request(request) {
	(void)scriptPath;
}
