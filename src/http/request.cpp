#include "request.hpp"

/* MAIN */

Request::Request(void) {}
Request::Request(ListenSocket &listener, Config &server_config) {
	(void)server_config;
	// INIT //

	this->_statusCode = 0;

	std::string buffer = listener.getBuffer();
	std::vector<std::string> lines = getLines(buffer);

	// REQUEST LINE //

	std::vector<std::string> words = splitString(lines.at(0));
	std::string method = words.at(0), url = words.at(1), protocol = words.at(2);

	this->_url = url;

	if (methodValid(method)) { this->_method = method; } else { this->_statusCode = 400; return; }
	if (protocolValid(protocol)) { this->_protocol = protocol; } else { this->_statusCode = 400; return; }
	//if (this->_url.substr())

	if(this->_url.find("/cgi-bin/")) {
		this->_cgiEnabled = true;
	}

	// HEADERS //

	size_t body_line = 0;
	for (size_t i = 0; i < lines.size(); i++) {
		if (lines.at(i) == "\n") { body_line = i; break; }
		this->_formatHeader(lines.at(i));
		//body_line = i;
	}

	//const std::string root_page = server_config.getLocationRoot("/");
	//this->_handleFileRequest(root_page, url);

	// BODY //

	std::cout << std::endl << "----------------------------" << std::endl;

	for (size_t i = body_line; i < lines.size(); i++) {
		//std::cout << lines.at(i) << std::endl;
	}
		/*! MAKE BODY GESTION FOR INTERACTIVE METHODS !*/

	// IF EVERYTHING NO PROCEED = EVERYTHING OK

	return;
}

bool Request::_formatHeader(const std::string &headerLine) {
	int collon = headerLine.find(':');

	std::string index;
	std::string value;

	if (collon > 1) {
		index = headerLine.substr(0, collon);
		value = headerLine.substr(collon + 1, headerLine.length());
	} else { return false; }

	if (index.length() <= 0 || value.length() <= 0) { return false; }

	for (size_t i = 0; i <= value.length(); i++) {
		if (isspace(value[i])) { continue; } else {
			value = value.substr(i, value.length() - 1);
			break;
		}
	}

	if (value.length() <= 0) { return false; }
	this->_headers[index] = value;

	return(true);
}

// GETTERS

int Request::getStatusCode(void) const { return this->_statusCode; }
std::string Request::getMethod(void) const { return this->_method; }
std::string Request::getUrl(void) const { return this->_url; }
std::string Request::getProtocol(void) const { return this->_protocol; }

std::map<std::string, std::string> Request::getHeaders(void) const { return this->_headers; }
