#include "response.hpp"

Response::Response(void) {}
Response::Response(Request &request) {
	struct parsing parsingResult = request.getParsing();

	// RESPONSE LINE
	this->_response = request.getProtocol() + " " + ft_itoa(parsingResult.status_code) + " " + parsingResult.status_message + "\n";

	// HEADERS
	this->_response += "Date: " + getTime() + "\n";
	this->_response += "Server: server\n";

	this->_processRequest(request.getMethod(), request);
}

Response::~Response( void ) {
	this->_response = "";
}

void Response::_processRequest(std::string method, Request &request) {
	if (method.compare("GET") == 0) {
		Get methodResult(request);
		methodResult.process(request);
		this->_response += "Content-Type: " + getContentType(request.getUrl()) + "\n";
		this->_response += "Content-Length: " + ft_itoa(methodResult.getContentLength()) + "\n";
		this->_response += "\n";
		this->_response += methodResult.getContent();
	}
}

std::string Response::getResponse(void) {
	return (this->_response);
}
