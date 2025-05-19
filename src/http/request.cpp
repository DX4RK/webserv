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

	// FIRST LINE HANDLING

	try {
		std::vector<std::string> words = splitString(lines.at(0));
		std::string method = words.at(0), url = words.at(1), protocol = words.at(2);

		if (methodValid(method)) { this->_method = method; } else {
			this->_parsingResult.status_code = 400;
			this->_parsingResult.proceed = true;
			this->_parsingResult.status_message = "invalid " + method + " method.";
			throw std::exception();
		}

		if (protocolValid(protocol)) { this->_protocol = protocol; } else {
			this->_parsingResult.status_code = 400;
			this->_parsingResult.proceed = true;
			this->_parsingResult.status_message = "invalid protocol version.";
			throw std::exception();
		}
	} catch ( std::exception &e ) {
		std::cout << "Error: " << this->_parsingResult.status_code << std::endl;
		std::cout << this->_parsingResult.status_message << std::endl;
	}

}
