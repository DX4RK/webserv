#include "response.hpp"

Response::Response(void) {}
Response::Response(Request &request) {
	struct parsing parsingResult = request.getParsing();

	// RESPONSE LINE
	this->_response = request.getProtocol() + " " + ft_itoa(parsingResult.status_code) + " " + parsingResult.status_message + "\n";

	// HEADERS
	this->_headers += "Server: webserv/1.0\n";
	this->_headers += "Date: " + getTime() + "\n";

	if (parsingResult.method_allowed) {
		Method methodResult = this->_processRequest(request.getMethod(), request);

		this->_response += this->_headers += "\n";
		this->_response += methodResult.getContent();
	}

}

Response::~Response( void ) {
	this->_response = "";
}

Method Response::_processRequest(std::string method, Request &request) {
	if (method.compare("GET") == 0) {
		Get methodResult(request);
		methodResult.process(*this, request);
		//this->_response += "\n";
		//this->_response += methodResult.getContent();
		return methodResult;
	}
	Get methodResult(request);
	methodResult.process(*this, request);
	//this->_response += "\n";
	//this->_response += methodResult.getContent();
	return methodResult;
}

std::string Response::getResponse(void) {
	return (this->_response);
}

void Response::addHeader(std::string headerName, std::string headerValue) {
	this->_headers += headerName + ": " + headerValue + "\n";
}
