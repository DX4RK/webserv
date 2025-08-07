#include "request.hpp"
#include <filesystem>

/* UTILS */

std::string readChunkedBody(const std::vector<std::string>& lines, size_t start) {
	std::string body = "";
	size_t i = start;
	while (i < lines.size()) {
		std::string line = trim(lines[i++], false);
		if (line.empty()) continue;

		// Convert chunk size from hex using stringstream
		std::istringstream iss(line);
		int chunkSize = 0;
		iss >> std::hex >> chunkSize;
		if (chunkSize == 0) break;

		// Read the chunk data (can span multiple lines depending on how you split)
		if (i >= lines.size()) break;

		// Collect chunkSize bytes (rough version assuming 1 line = 1 chunk)
		std::string chunkLine = lines[i++];
		if ((int)chunkLine.size() >= chunkSize)
			body += chunkLine.substr(0, chunkSize);
		else
			body += chunkLine; // fallback

		// After each chunk, there should be a \r\n line separator — skip it if needed
	}
	return body;
}

/* MAIN */

Request::Request(void) {}
Request::Request(ListenSocket &listener, Config *config, int errorCode) {

	std::cout << LIGHT_BLUE << BOLD << "==============[webserv]==============" << RESET << std::endl;

	// INIT //

	this->_body = "";
	this->_statusCode = 0;
	this->server_config = config;
	this->_currentPort = getCurrentPort(listener.getNewSocket());

	if (errorCode != 0) {
		this->_protocol = "HTTP/1.1";
		this->_statusCode = errorCode;
		return;
	}

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

	// HEADERS //

	size_t body_line = 0;
	for (size_t i = 0; i < lines.size(); ++i) {
		std::string line = lines.at(i);
		if (line == "" || line == "\r") { body_line = i + 1; break; }
		this->_formatHeader(line);

	}

	// BODY //

	bool isChunked = false;
	try {
		std::string headerValue = trim(this->findHeader("Transfer-Encoding"), false);
		if (headerValue == "chunked") {
			isChunked = true;}
	} catch (std::exception &e) {}

	if (isChunked) {
		this->_body = readChunkedBody(lines, body_line);
	} else {
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
	}

	// URL HANDLING //

	std::string location;
	std::string path;
	std::string refererUrl;
	std::string originalUrl = url;

	bool directory = false;

	if (url.at(url.length() - 1) == '/')
		directory = true;

	try {
		std::string host = this->findHeader("Host");
		std::string referer = trim(this->findHeader("Referer"), false);

		referer = extractPath(referer);
		if (!(referer.length() == 1 && referer.at(0) == '/'))
			refererUrl = referer;
	} catch (std::exception &e) {}

	locationConfig locationConfig = this->server_config->getLocationFromPath(url);
	if (!refererUrl.empty() && isDirectory(locationConfig.root + refererUrl)) {
		url = refererUrl + url;
		locationConfig = this->server_config->getLocationFromPath(url);
	}

	std::string root = locationConfig.root;

	size_t urlFind = url.find(locationConfig.path);
	if (urlFind != std::string::npos) {
		size_t pathLength = locationConfig.path.length();
		if (pathLength == url.length())
			url = "";
		else
			url = url.substr(pathLength);
	}

	if (url.empty())
		directory = true;
	else if (url.at(0) != '/')
		url = "/" + url;

	path = root + url;

	this->_path = path;
	this->_location = locationConfig.path;

	// CGI CHECK

	size_t extensionEnd = 0;
	this->_cgiEnabled = false;
	if (!directory && locationConfig.cgi_extension.size() > 0) {
		std::string fileExtension;
		size_t dotPosition = path.find_last_of('.');
		if (dotPosition != std::string::npos) {
			extensionEnd = path.find_first_of('/', dotPosition);
			if (extensionEnd == std::string::npos)
				extensionEnd = path.length();
			fileExtension = path.substr(dotPosition, extensionEnd - dotPosition);
		}
		if (!fileExtension.empty()) {
			for (size_t i = 0; i < locationConfig.cgi_extension.size(); i++) {
				if (locationConfig.cgi_extension.at(i) == fileExtension) {
					this->_cgiEnabled = true;
					this->_cgiExtension = fileExtension;
					break;
				}
			}
		}
	}
	// for tester
	this->_pathInfo = originalUrl;
	/*if (this->_cgiEnabled && extensionEnd < path.length()) {
		this->_pathInfo = path.substr(extensionEnd);
		this->_path = path.substr(0, extensionEnd);
	}*/
	std::cout << "Path info: " << this->_pathInfo << std::endl;
	// LOG

	std::cout << "Path: " << path << std::endl;
	std::cout << "Directory: " << directory << std::endl;
	std::cout << "Location: " << locationConfig.path << std::endl;

	// CHECK

	if (this->server_config->getLocationFromPath(this->_location).client_max_body_size < this->_body.length()) {
		this->_statusCode = 413;
		return;
	}

	if (this->_cgiEnabled) {
		if (!this->server_config->isCgiMethodAllowed(this->_location, method)) {
			this->_cgiEnabled = false;
			return;
		}
	} else {
		if (!this->server_config->isMethodAllowed(this->_location, method)) {
			this->_statusCode = 405;
			return;
		}
	}

	(void)directory;
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
int Request::getServerPort(void) const { return this->_currentPort; }
bool Request::isCgiEnabled( void ) const { return this->_cgiEnabled; }
bool Request::isReqDirectory( void ) const { return this->_isDirectory; }

std::string Request::getMethod(void) const { return this->_method; }
std::string Request::getUrl(void) const { return this->_url; }
std::string Request::getOriginalUrl(void) const { return this->_originalUrl; }
std::string Request::getPath(void) const { return this->_path; }
std::string Request::getLocation(void) const { return this->_location; }
std::string Request::getFileName(void) const { return this->_fileName; }
std::string Request::getBody(void) const { return this->_body; }
std::string Request::getCgiExtension( void ) const { return this->_cgiExtension; }
std::string Request::getProtocol(void) const { return this->_protocol; }
std::string Request::getPathInfo(void) const { return this->_pathInfo; }

std::map<std::string, std::string> Request::getHeaders(void) const { return this->_headers; }

std::string Request::findHeader( std::string index ) {
	std::map<std::string, std::string>::const_iterator it = this->_headers.find(index);

	if (it != this->_headers.end()) return (it->second);
	throw std::exception();
}
