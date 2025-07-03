#include "request.hpp"

/* UTILS */

std::string getLocatin(std::string url) {
	std::size_t firstSlash = url.find_first_of('/');
	std::size_t lastSlash = url.find_last_of('/');

	if (firstSlash == std::string::npos)
		return "/";
	if (lastSlash == std::string::npos || lastSlash == 0) {
		if (url.size() > 1) {
			return (url.substr(firstSlash, url.size()));
		}
		return "/";
	}
	return (url.substr(firstSlash, lastSlash));
}

/* MAIN */

Request::Request(void) {}
Request::Request(ListenSocket &listener, Config *config) {

	// INIT //

	this->_body = "";
	this->_statusCode = 0;
	this->server_config = config;

	std::string buffer = listener.getBuffer();
	std::vector<std::string> lines = getLines(buffer);

	std::vector<std::string> words = splitString(lines.at(0));

	if (words.size() < 3)
	{
		this->_statusCode = 400;
		return;
	}

	std::string method = words.at(0), url = words.at(1), protocol = words.at(2);

	if (methodValid(method)) { this->_method = method; } else { this->_statusCode = 400; return; }
	if (protocolValid(protocol)) { this->_protocol = protocol; } else { this->_statusCode = 400; return; }

	if(this->_url.find("/cgi-bin/") != std::string::npos) { this->_cgiEnabled = true; }

	//if (!this->server_config->isLocationMethodsAllowed(findPath(this->_url), method)) {
	//	this->_statusCode = 405;
	//	return;
	//};

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

	// URL HANDLING //

	const std::string root = this->server_config->getLocationRoot("/");

	std::map<std::string, std::string>::const_iterator it = this->getHeaders().find("Referer");

	// TODO: make this handle special case like trying to fetch style.css on http://localhost:8080/default/index.html, path become /default/index.html/default/style.css

	if (it != this->getHeaders().end()) {
		std::string referer = extractPath(this->getHeaders()["Referer"]);
		url = trim(referer, false) + trim(url, false);
	}

	std::string path = root + url;

	std::cout << "Url: " << url << std::endl;
	std::cout << "Path: " << path << std::endl;

	this->_url = url;
	this->_path = path;
	this->_location = getLocatin(url);
	std::cout << "Location: " << this->_location << std::endl;
	if (!this->server_config->isLocationMethodsAllowed(this->_location, method)) {
		this->_statusCode = 405;
	std::cout << "----------------" << std::endl;

		return;
	};
	std::cout << "----------------" << std::endl;


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
std::string Request::getPath(void) const { return this->_path; }
std::string Request::getLocation(void) const { return this->_location; }
std::string Request::getBody(void) const { return this->_body; }
std::string Request::getProtocol(void) const { return this->_protocol; }

std::map<std::string, std::string> Request::getHeaders(void) const { return this->_headers; }

std::string Request::findHeader( std::string index ) {
	std::map<std::string, std::string>::const_iterator it = this->_headers.find(index);

	if (it != this->_headers.end()) return (it->second);
	throw std::exception();
}
