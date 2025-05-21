#include "request.hpp"

/* MAIN */

Request::Request(void) {}
Request::Request(ListenSocket &listener) {

	// INIT //

	this->_parsingResult.proceed = false;
	this->_parsingResult.status_code = 0;
	this->_parsingResult.status_message = "";

	std::string buffer = listener.getBuffer();
	std::vector<std::string> lines = getLines(buffer);

	// REQUEST LINE //

	try {
		std::vector<std::string> words = splitString(lines.at(0));
		std::string method = words.at(0), url = words.at(1), protocol = words.at(2);

		if (methodValid(method)) { this->_method = method; } else {
			this->_parsingResult.status_code = 400;
			this->_parsingResult.proceed = true;
			this->_parsingResult.status_message = "Invalid " + method + " method";
			throw std::exception();
		}

		if (protocolValid(protocol)) { this->_protocol = protocol; } else {
			this->_parsingResult.status_code = 400;
			this->_parsingResult.proceed = true;
			this->_parsingResult.status_message = "Invalid protocol version";
			throw std::exception();
		}

		if (!this->_handleFileRequest(url)) { throw std::exception(); }

	} catch ( std::exception &e ) {
		//std::cout << "Error: " << this->_parsingResult.status_code << std::endl;
		//std::cout << this->_parsingResult.status_message << std::endl;
		return;
	}

	// HEADERS //

	try {
		for (size_t i = 0; i < lines.size(); i++) {
			if (lines.at(i) == "\n") { break; }
			if (this->_formatHeader(lines.at(i))) {
				this->_parsingResult.status_code = 400;
				this->_parsingResult.proceed = true;
				this->_parsingResult.status_message = "Invalid header " + i;
				throw std::exception();
			}
		}
	} catch ( std::exception &e ) {
		//std::cout << "Error: " << this->_parsingResult.status_code << std::endl;
		//std::cout << this->_parsingResult.status_message << std::endl;
		return;
	}

	// BODY //

		/*! MAKE BODY GESTION FOR INTERACTIVE METHODS !*/

	// IF EVERYTHING NO PROCEED = EVERYTHING OK

	if (!this->_parsingResult.proceed) {
		this->_parsingResult.proceed = true;
		this->_parsingResult.status_code = 200;
		this->_parsingResult.status_message = "OK";
	}

	// REQUEST DATA IS READY TO MAKE RESPONSE
}

bool Request::_handleFileRequest(const std::string &filePath) {
	std::string fullPath = WEB_ROOT + filePath;

	this->_fileName = getLastSub(fullPath, '/');

	if (!fileExists(fullPath)) {
		this->_parsingResult.proceed = true;
		this->_parsingResult.status_code = 404;
		this->_parsingResult.status_message = "File does not exist";
		return (false);
	}

	// Check if it’s a directory
	if (isDirectory(fullPath)) {
		fullPath += "/index.html"; // Try to resolve to index.html
		this->_fileName = "index";
		if (!fileExists(fullPath)) {
			this->_parsingResult.proceed = true;
			this->_parsingResult.status_code = 403;
			this->_parsingResult.status_message = "Directory without default page";
			return (false);
		}
	}

	// Check read permissions
	if (!hasReadPermission(fullPath)) {
		this->_parsingResult.proceed = true;
		this->_parsingResult.status_code = 403;
		this->_parsingResult.status_message = "No read permission";
		return (false);
	}

	this->_filePath = fullPath;
	return (true);
}

bool Request::_formatHeader(const std::string &headerLine) {
	int collon = headerLine.find(':');

	std::string index;
	std::string value;

	if (collon > 1) {
		index = headerLine.substr(0, collon - 1);
		value = headerLine.substr(collon + 1, headerLine.length());
	} else { return false; }

	if (index.length() <= 0 || value.length() <= 0 || value[value.length()] != '\n') { return false; }

	for (size_t i = 0; i <= value.length(); i++) {
		if (isspace(value[i])) { continue; } else {
			value = value.substr(i, value.length() - 1);
			break;
		}
	}

	if (value.length() <= 0) { return false; }

	return(true);
}

// GETTERS

std::string Request::getMethod(void) const { return this->_method; }
std::string Request::getUrl(void) const { return this->_filePath; }
std::string Request::getProtocol(void) const { return this->_protocol; }
std::string Request::getFileName(void) const { return this->_fileName; }

struct parsing Request::getParsing( void ) const { return this->_parsingResult; }
std::map<std::string, std::string> Request::getHeaders(void) const { return this->_headers; }
