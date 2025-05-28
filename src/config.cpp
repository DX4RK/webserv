#include "config.hpp"

void extractData(std::string line, std::map<std::string, std::string> *map);
void setDataFromFile(std::string path, std::map<std::string, std::string> *map);

Config::Config(std::string configPath) {

	(void)configPath;

	this->_port = 8080;
	this->_serverName = "localhost";

	locationConfig root_config;
	root_config.path = "/";
	root_config.root = "./www";
	root_config.index.push_back("index.html");

	this->_locations[root_config.path] = root_config;

	setDataFromFile(MIME_TYPES_PATH, &this->_mimes);
	setDataFromFile(CODE_STATUS_PATH, &this->_codeStatus);
}

std::string Config::getContentType(const std::string& fileName) {
	std::string type = getLastSub(fileName, '.');
	std::map<std::string, std::string>::iterator it = this->_mimes.find(type);
	if (it != this->_mimes.end()) {
		return it->second;
	}
	return "application/octet-stream";
}

std::string Config::getStatusCode(const std::string& code) {
	std::map<std::string, std::string>::iterator it = this->_codeStatus.find(code);
	if (it != this->_codeStatus.end()) {
		return it->second;
	}
	return "???";
}

/* UTILS */

/* GETTERS & SETTERS */

int Config::getPort( void ) const { return this->_port; }

std::string Config::getLocationRoot( std::string path ) {
	std::map<std::string, location_config>::const_iterator it = this->_locations.find(path);
	if (it != this->_locations.end()) { return it->second.root; }
	return "./www";
}

void setDataFromFile(std::string path, std::map<std::string, std::string> *map) {
	std::string file_path = path;
	std::map<std::string, std::string> config;

	int fd = open(file_path.c_str(), O_RDONLY);
	char tempBuffer[30000] = {0};
	ssize_t bytesRead = read(fd, tempBuffer, 30000);
	if (bytesRead < 0) {
		close(fd);
		return;
	}

	std::string content = std::string(tempBuffer, bytesRead);
	close(fd);

	std::vector<std::string> lines = getLines(content);

	for (size_t i = 0; i < lines.size(); i++) {
		if (i < 1 || i >= lines.size() - 2) continue;
		extractData(lines.at(i), map);
	}
}

void extractData(std::string line, std::map<std::string, std::string> *map) {
	std::string value;

	int end = 0;
	int start = skip_space(line);

	for (size_t i = (size_t)start; i < line.length(); i++) {
		if (isspace(line[i]) && isspace(line[i + 1])) { end = (int)i; break; }
	}

	value = line.substr(start, end - start);
	line = line.substr(end, line.size() - end);

	for (size_t i = (size_t)start; i < line.length(); i++) {
		if (!isspace(line[i])) { line = line.substr(i, line.size() - (i + 1) ); break; }
	}

	std::vector<std::string> indexes = splitString(line);

	for (size_t i = 0; i < indexes.size(); i++) {
		(*map)[indexes.at(i)] = value;
	}

	return ;
}
