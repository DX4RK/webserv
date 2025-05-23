#include "request.hpp"

/* MAIN */

Request::Request(void) {}
Request::Request(ListenSocket &listener, Config &server_config) {

	// INIT //

	this->_parsingResult.proceed = false;
	this->_parsingResult.status_code = 0;
	this->_parsingResult.status_message = "";
	this->_parsingResult.method_allowed = false;

	std::string buffer = listener.getBuffer();
	std::vector<std::string> lines = getLines(buffer);

	// REQUEST LINE //

	std::vector<std::string> words = splitString(lines.at(0));
	std::string method = words.at(0), url = words.at(1), protocol = words.at(2);

	try {
		if (methodValid(method)) { this->_method = method; } else {
			this->_parsingResult.status_code = 400;
			this->_parsingResult.proceed = true;
			this->_parsingResult.status_message = "Invalid " + method + " method";
		}

		if (protocolValid(protocol)) { this->_protocol = protocol; } else {
			this->_parsingResult.status_code = 400;
			this->_parsingResult.proceed = true;
			this->_parsingResult.status_message = "Invalid protocol version";
		}
	} catch ( std::exception &e ) {
		std::cout << "Server crashed." << std::endl;
		return;
	}

	// HEADERS //

	try {
		for (size_t i = 0; i < lines.size(); i++) {
			if (lines.at(i) == "\n") { break; }
			this->_formatHeader(lines.at(i));
		}
	} catch ( std::exception &e ) {
		std::cout << "Server crashed." << std::endl;
		return;
	}

	const std::string root_page = server_config.getLocationRoot("/");
	this->_handleFileRequest(root_page, url);

	// BODY //

		/*! MAKE BODY GESTION FOR INTERACTIVE METHODS !*/

	// IF EVERYTHING NO PROCEED = EVERYTHING OK

	//for (std::map<std::string, std::string>::iterator it = this->_headers.begin(); it != this->_headers.end(); ++it) {
   //   std::cout << it->first << ": " << it->second << std::endl;
    //}

	//if (this->_headers.) {
	//	std::cout << "uhm!" << std::endl;
	//}

	//std::cout << this->_headers["Accept"] << std::endl;

	if (!this->_parsingResult.proceed) {
		this->_parsingResult.proceed = true;
		this->_parsingResult.status_code = 200;
		this->_parsingResult.status_message = "OK";
		this->_parsingResult.method_allowed = true;
	}

	// REQUEST DATA IS READY TO MAKE RESPONSE
}

bool Request::_handleFileRequest(const std::string root, const std::string &filePath) {
	std::string fullPath = root + filePath;

	std::map<std::string, std::string>::const_iterator it = this->_headers.find("Referer");

	if (it != this->_headers.end()) {
		std::string path = extractPath(this->_headers["Referer"]);
		std::cout << path << std::endl;
		fullPath = WEB_ROOT + trim(path) + trim(filePath);
	}

	//std::cout << this

	this->_fileName = getLastSub(fullPath, '/');

	if (!fileExists(fullPath)) {
		this->_parsingResult.proceed = true;
		this->_parsingResult.status_code = 404;
		this->_parsingResult.method_allowed = true;
		this->_parsingResult.status_message = "Not Found";

		this->_method = "GET";
		this->_filePath = NOT_FOUND_PAGE;
		this->_fileName = getLastSub(fullPath, '/');
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
		this->_parsingResult.status_message = "No Premissions";
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

std::string Request::getMethod(void) const { return this->_method; }
std::string Request::getUrl(void) const { return this->_filePath; }
std::string Request::getProtocol(void) const { return this->_protocol; }
std::string Request::getFileName(void) const { return this->_fileName; }

struct parsing Request::getParsing( void ) const { return this->_parsingResult; }
std::map<std::string, std::string> Request::getHeaders(void) const { return this->_headers; }
