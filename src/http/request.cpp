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

	std::cout << LIGHT_BLUE << BOLD << "==============[webserv]==============" << RESET << std::endl;

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

	// BODY //

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

	bool rootUrl = false;
	bool directory = false;

	std::string root;
	std::string fileName;
	std::string location;
	std::string referer;
	std::string originalLocation;
	std::string formatUrl = url;
	std::string originalUrl = url;

	formatUrl = getWithoutSlashes(url);
	if (formatUrl == "") {
		formatUrl = "/";
		rootUrl = true;
	}

	size_t dot_pos = formatUrl.find('.');
	size_t lastSlash = formatUrl.find_last_of('/');

	if ((dot_pos != std::string::npos) && dot_pos > lastSlash) {
		directory = false;
		if (lastSlash != std::string::npos) {
			location = formatUrl.substr(0, lastSlash);
			fileName = formatUrl.substr(lastSlash + 1, formatUrl.length());
		} else {
			location = "";
			fileName = formatUrl;
		}
	} else {
		if ((dot_pos != std::string::npos) && (lastSlash == std::string::npos)) {
			location = "";
			fileName = formatUrl;
		} else {
			directory = true;
			location = formatUrl;
			fileName = "";
		}
	}

	try {
		root = this->server_config->getLocationRoot(location);
		root = getWithoutSlashes(root);

		size_t lastSlash = root.find_last_of('/');
		size_t slashCount = std::count(root.begin(), root.end(), '/');

		if (lastSlash != std::string::npos) {
			if (slashCount > 1) {
				location = root.substr(lastSlash, root.length());
				root = root.substr(0, lastSlash);
			}
		} else
			throw std::exception();
	} catch (std::exception &e) {
		root = this->server_config->getLocationRoot("/");
	}

	std::string path;
	originalLocation = location;
	if (!location.empty() && location.at(0) != '/')
		location = "/" + location;

	try {
		referer = trim(this->findHeader("Referer"), false);
		referer = extractPath(referer);

		std::string testPath = root + referer + location;
		if (isDirectory(testPath)) {
			size_t firstSlashPos = referer.find_first_of('/');
			if (firstSlashPos != std::string::npos)
				referer = referer.substr(firstSlashPos + 1);
			if (referer.length() > 0) {
				size_t findReferer = originalLocation.find(referer);
				if (findReferer == std::string::npos) {
					location = referer + originalLocation;
				}
			}
		}
	} catch (std::exception &e) {}

	(void)rootUrl;
	(void)directory;

	if (!location.empty() && location.at(0) != '/')
		location = "/" + location;

	if (fileName.empty() && lastSlash != std::string::npos) {
		std::string mainPath = root + location + "/" + fileName;
			std::cout << "here" << std::endl;

		if (mainPath.at(mainPath.length() - 1) == '/') {
			mainPath = mainPath.substr(0, mainPath.length() - 1);
		}

		if (!isDirectory(mainPath)) {
			directory = false;
			size_t mainLastSlash = mainPath.find_last_of("/");
			std::cout << "here" << std::endl;
			if (mainLastSlash < mainPath.length()) {
				fileName = mainPath.substr(mainLastSlash + 1);
			}
			std::cout << location << std::endl;
			std::cout << "yessss" << std::endl;
			size_t locationLastSlash = location.find_last_of("/");
			if (locationLastSlash < location.length()) {
				location = location.substr(0, locationLastSlash);
			}
		}


	}

	//this->_statusCode = 500;

	this->_url = formatUrl;
	this->_fileName = fileName;
	this->_isDirectory = directory;
	this->_path = root + location + "/" + fileName;
	this->_location = location;
	this->_originalUrl = originalUrl;

	// check

	if (this->server_config->getLocationFromPath(this->_location).client_max_body_size < this->_body.length()) {
		this->_statusCode = 413;
		return;
	}

	if (!this->server_config->isMethodAllowed(this->_location, method)) {
		this->_statusCode = 405;
		return;
	}
	// Logging
	std::cout << "Url: " << this->_url << std::endl;
	std::cout << "Original Url: " << this->_originalUrl << std::endl;
	std::cout << "Root: " << root << std::endl;
	std::cout << "Path: " << this->_path << std::endl;
	std::cout << "Location: " << this->_location << std::endl;
	std::cout << "File Name: " << fileName << std::endl;
	//std::cout << "Location Config Path: " << this->server_config->getLocationFromPath(this->_location).path << std::endl;
	std::cout << std::endl << BOLD << "--------------------------------" << RESET << std::endl << std::endl;

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
bool Request::isReqDirectory( void ) const { return this->_isDirectory; }

std::string Request::getMethod(void) const { return this->_method; }
std::string Request::getUrl(void) const { return this->_url; }
std::string Request::getOriginalUrl(void) const { return this->_originalUrl; }
std::string Request::getPath(void) const { return this->_path; }
std::string Request::getLocation(void) const { return this->_location; }
std::string Request::getFileName(void) const { return this->_fileName; }
std::string Request::getBody(void) const { return this->_body; }
std::string Request::getProtocol(void) const { return this->_protocol; }

std::map<std::string, std::string> Request::getHeaders(void) const { return this->_headers; }

std::string Request::findHeader( std::string index ) {
	std::map<std::string, std::string>::const_iterator it = this->_headers.find(index);

	if (it != this->_headers.end()) return (it->second);
	throw std::exception();
}
