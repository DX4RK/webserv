#include "response.hpp"

Response::Response(void) {}
Response::Response(Request &request, Config &server_config) {
	//struct parsing parsingResult = request.getParsing();
	this->_responseCode = request.getStatusCode();

	std::string methodHeaders = "";
	std::string methodContent = "";

	this->addHeader("Server", "webserv/1.0");
	this->addHeader("Date", getTime());

	if (this->_responseCode == 0) {
		Method methodResult = this->_processRequest(request.getMethod(), request, server_config);
		this->_responseCode = methodResult.getReturnCode();

		if (this->_responseCode == 0) { this->_responseCode = 500; return; }

		methodContent = methodResult.getContent();
	}

	// RESPONSE LINE

	std::string responseCode = ft_itoa(this->_responseCode);
	std::string responseMessage = server_config.getStatusCode(responseCode);

	this->_response = request.getProtocol() + " " + responseCode + " " + responseMessage + "\n";
	this->_response += this->_headers + "\n" + methodContent;
}

Response::~Response( void ) {
	this->_response = "";
}

Method Response::_processRequest(std::string method, Request &request, Config &server_config) {
	if (method.compare("GET") == 0) {
		Get methodResult(request, server_config);
		methodResult.process(*this, request, server_config);
		return methodResult;
	} else if (method.compare("POST") == 0) {
		Post methodResult(request, server_config);
		methodResult.process(*this, request, server_config);
		return methodResult;
	}

	Method methodResult(request, server_config);
	return methodResult;
}

void Response::addHeader(std::string headerName, std::string headerValue) {
	this->_headers += trim(headerName, false) + ": " + trim(headerValue, false) + "\n";
}

/* GETTERS & SETTERS */

int Response::getResponseCode(void) const { return (this->_responseCode); }
std::string Response::getResponse(void) const { return (this->_response); }
