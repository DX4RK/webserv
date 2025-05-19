#include "response.hpp"

Response::Response(void) {}
Response::Response(Request &request) {
	struct parsing parsingResult = request.getParsing();

	// RESPONSE LINE
	this->_response = request.getProtocol() + " " + ft_itoa(parsingResult.status_code) + " " + parsingResult.status_message + "\n";

	// HEADERS
	this->_response += "Date: " + getTime() + "\n";

	std::cout << this->_response;
}
