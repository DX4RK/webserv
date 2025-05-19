#include "../../includes/request.hpp"

//std::vector<std::string> validMethods;
//validMethods.push_back("GET"); validMethods.push_back("POST"); validMethods.push_back("PUT");

//bool in_array(const std::string &value, const std::vector<std::string> &array)
//{
//	return std::find(array.begin(), array.end(), value) != array.end();
//}

/* UTILS */

bool fileExists(const std::string &filePath) {
	struct stat buffer;
	return (stat(filePath.c_str(), &buffer) == 0);
}

bool isDirectory(const std::string &path) {
	struct stat buffer;
	if (stat(path.c_str(), &buffer) != 0) {
		return false; // Path does not exist
	}
	return S_ISDIR(buffer.st_mode);
}

bool hasReadPermission(const std::string &filePath) {
	return (access(filePath.c_str(), R_OK) == 0);
}

bool methodValid(std::string method) {
	std::string methods[3] = { "GET", "POST", "PUT" };
	for (size_t i = 0; i < methods->length(); i++) {
		if (methods[i] == method) { return true; }
	}
	return false;
}

bool protocolValid(std::string protocol) {
	if (protocol.substr(0, 4).compare("HTTP") != 0) { return false; }
	if (protocol[4] != '/') { return false; }
	if (protocol.substr(5, 3).compare("1.1") != 0) { return false; }

	return (true);
}

std::vector<std::string> getLines(std::string buffer) {
	std::string line;
	std::vector<std::string> lines;

	for (size_t i = 0; i <= buffer.length(); i++) {
		if (buffer[i] == '\n') {
			lines.push_back(line); line = "";
			continue;
		}
		line = line + buffer[i];
	}
	if (line.length() > 0) { lines.push_back(line); line = ""; }

	return (lines);
}

std::vector<std::string> splitString(std::string str) {
	std::stringstream ss(str);
	std::string word;
	std::vector<std::string> words;

	while (ss >> word) { words.push_back(word); }
	return (words);
}

/* MAIN */

Request::Request(void) {}
Request::Request(ListenSocket &listener) {
	//std::vector<std::string>
	this->_parsingResult.proceed = false;
	this->_parsingResult.status_code = 0;
	this->_parsingResult.status_message = "";

	std::string buffer = listener.getBuffer();
	std::vector<std::string> lines = getLines(buffer);

	// REQUEST LINE

	std::cout << lines.at(0) << std::endl;

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
		std::cout << "Error: " << this->_parsingResult.status_code << std::endl;
		std::cout << this->_parsingResult.status_message << std::endl;
		return;
	}

	// HEADERS

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
		std::cout << "Error: " << this->_parsingResult.status_code << std::endl;
		std::cout << this->_parsingResult.status_message << std::endl;
		return;
	}

	// BODY

	// END

	this->_parsingResult.proceed = true;
	this->_parsingResult.status_code = 200;
	this->_parsingResult.status_message = "OK";

	Response t(*this);
}

bool Request::_handleFileRequest(const std::string &filePath) {
	std::string fullPath = WEB_ROOT + filePath;

	if (!fileExists(fullPath)) {
		this->_parsingResult.proceed = true;
		this->_parsingResult.status_code = 404;
		this->_parsingResult.status_message = "File does not exist";
		return (false);
	}

	// Check if it’s a directory
	if (isDirectory(fullPath)) {
		fullPath += "/index.html"; // Try to resolve to index.html
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
std::string Request::getUrl(void) const { return this->_url; }
std::string Request::getProtocol(void) const { return this->_protocol; }

struct parsing Request::getParsing( void ) const { return this->_parsingResult; }
std::map<std::string, std::string> Request::getHeaders(void) const { return this->_headers; }
