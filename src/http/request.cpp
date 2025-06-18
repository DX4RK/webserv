#include "request.hpp"

/* MAIN */

Request::Request(void) {}
Request::Request(ListenSocket &listener, Config *config) {

	// INIT //

	this->_body = "";
	this->_statusCode = 0;
	this->server_config = config;

	std::string buffer = listener.getBuffer();
	std::vector<std::string> lines = getLines(buffer);

	// aborted crash fixed
	std::vector<std::string> words = splitString(lines.at(0));

	if (words.size() < 3)
	{
		this->_statusCode = 400;
		return;
	}
	std::string method = words.at(0), url = words.at(1), protocol = words.at(2);

	// Gestion GitHub callback login
	if (url.find("/github/callback") == 0) {
		this->_url = url;
		this->_method = method;
		this->_protocol = protocol;
		this->_cgiEnabled = false;
		this->_statusCode = 0;

		// Headers parsing
		size_t body_line = 0;
		for (size_t i = 1; i < lines.size(); ++i) {
			std::string line = lines.at(i);
			if (line == "" || line == "\r") { body_line = i + 1; break; }
			this->_formatHeader(line);
		}

		size_t body_end = lines.size();
		if (lines.size() > body_line) {
			bool isMultipart = false;
			std::map<std::string, std::string>::iterator it;
			for (it = this->_headers.begin(); it != this->_headers.end(); ++it) {
				if (it->first == "Content-Type" && it->second.find("multipart/") != std::string::npos) {
					isMultipart = true;
					break;
				}
			}

			for (size_t i = body_line; i < body_end; i++) {
				std::string jump = "\n";
				std::string line = lines.at(i);

				if (i + 1 >= body_end) { jump = ""; }

				if (isMultipart) {
					this->_body += line + jump;
				} else {
					this->_body += trim(line, false) + jump;
				}
			}
		}
		return;
	}

	this->_url = url;

	if (methodValid(method)) { this->_method = method; } else { this->_statusCode = 400; return; }
	if (protocolValid(protocol)) { this->_protocol = protocol; } else { this->_statusCode = 400; return; }

	if(this->_url.find("/cgi-bin/") != std::string::npos) { this->_cgiEnabled = true; }

	// HEADERS //

	size_t body_line = 0;
	for (size_t i = 0; i < lines.size(); ++i) {
		std::string line = lines.at(i);
		if (line == "" || line == "\r") { body_line = i + 1; break; }
		this->_formatHeader(line);

	}

	size_t body_end = lines.size();
	if (lines.size() > body_line) {
		bool isMultipart = false;
		std::map<std::string, std::string>::iterator it;
		for (it = this->_headers.begin(); it != this->_headers.end(); ++it) {
			if (it->first == "Content-Type" && it->second.find("multipart/") != std::string::npos) {
				isMultipart = true;
				break;
			}
		}

		for (size_t i = body_line; i < body_end; i++) {
			std::string jump = "\n";
			std::string line = lines.at(i);

			if (i + 1 >= body_end) { jump = ""; }

			if (isMultipart) {
				this->_body += line + jump;
			} else {
				this->_body += trim(line, false) + jump;
			}
		}
	}

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
bool Request::isCgiEnabled( void ) const { return this->_cgiEnabled; }

std::string Request::getMethod(void) const { return this->_method; }
std::string Request::getUrl(void) const { return this->_url; }
std::string Request::getBody(void) const { return this->_body; }
std::string Request::getProtocol(void) const { return this->_protocol; }

std::map<std::string, std::string> Request::getHeaders(void) const { return this->_headers; }

std::string Request::findHeader( std::string index ) {
	std::map<std::string, std::string>::const_iterator it = this->_headers.find(index);

	if (it != this->_headers.end()) return (it->second);
	throw std::exception();
}
