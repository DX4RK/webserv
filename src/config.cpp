#include "config.hpp"

/* UTILS */

bool is_digits(const std::string& str) {
	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
		if (!std::isdigit(*it)) {
			return false;
		}
	}
	return !str.empty();
}

int ft_atoi(std::string str) {
	std::stringstream ss(str);
	int num;
	ss >> num;
	return (num);
}

std::vector<std::string> separateString(std::string str) {
	std::istringstream iss(str);
	std::vector<std::string> result;
	std::string word;

	while (iss >> word)
		result.push_back(word);

	return (result);
}

std::string& trimStart(std::string& s) {
	size_t start = s.find_first_not_of(" \t\n\r\f\v");
	if (start == std::string::npos) {
		s.clear();
		return s;
	}
	s.erase(0, start);
	return s;
}

bool isBlock(std::string line) {
	if ((line.length() < 1))
		return false;

	std::vector<std::string> words = separateString(line);

	if ((words.size() < 2 || words.at(words.size() - 1) != "{"))
		return false;

	return true;
}

bool isIndexedBlock(std::string index, std::string line) {
	if ((line.length() < index.length()))
		return false;

	std::vector<std::string> words = separateString(line);
	if ((words.size() < 2) || (words.at(0) != index || words.at(words.size() - 1) != "{"))
		return false;

	return true;
}

value_config getValue(std::string line) {
	if (line.find('{') == 0 || line.find('}') == 0) {
		throw std::exception();
	}

	if (line.length() < 1) {
		value_config mainData;
		mainData.index = "";
		return mainData;
	} else {
		size_t endFind = line.find_last_of(';');

		if (endFind == std::string::npos || endFind == 0)
			throw std::exception();
		line = line.substr(0, endFind);
	}

	std::vector<std::string> words = separateString(line);

	if (words.size() < 2) {
		throw std::exception();
	}

	value_config mainData;

	mainData.index = words.at(0);
	words.erase(words.begin());
	mainData.values = words;

	return mainData;
}

size_t getBlockEnd(std::vector<std::string> lines, size_t start) {
	for (size_t index = start; index < (lines.size() - 1); index++) {
		std::string line = trimStart(lines.at(index));
		if (isBlock(line)) {
			throw std::exception();
		}
		if (line == "}")
			return index;
	}
	throw std::exception();
}

void addLocation(location_config *location, value_config *lineData) {
	if ((lineData->index == "return" && lineData->values.size() == 2) && is_digits(lineData->values.at(0))) {
		location->redirect_code = ft_atoi(lineData->values.at(0));
		location->redirect_url = lineData->values.at(1);
	} else if (lineData->index == "root" && lineData->values.size() == 1) {
		location->root = lineData->values.at(0);
	} else if (lineData->index == "autoindex" && lineData->values.size() == 1) {
		location->autoindex = (lineData->values.at(0) == "on");
	} else if (lineData->index == "listing" && lineData->values.size() == 1) {
		location->listing = (lineData->values.at(0) == "on");
	} else if (lineData->index == "client_max_body_size" && lineData->values.size() == 1) {
		location->client_max_body_size = ft_atoi(lineData->values.at(0));
	} else if (lineData->index == "cgi_extension") {
		location->cgi_extension = lineData->values;
	} else if (lineData->index == "index") {
		location->index = lineData->values;
	} else if (lineData->index == "allowed_methods") {
		location->allowed_methods = lineData->values;
	} else if (lineData->index == "upload_store" && lineData->values.size() == 1) {
		location->upload_store = lineData->values.at(0);
	}
}

std::map<int, std::string> createErrorPages(std::vector<std::string> lines, size_t start, size_t end) {
	std::map<int, std::string> main;

	main[200] = "./src/_default/errors/200.html";
	main[201] = "./src/_default/errors/201.html";
	main[204] = "./src/_default/errors/204.html";
	main[302] = "./src/_default/errors/302.html";
	main[400] = "./src/_default/errors/400.html";
	main[403] = "./src/_default/errors/403.html";
	main[404] = "./src/_default/errors/404.html";
	main[405] = "./src/_default/errors/405.html";
	main[411] = "./src/_default/errors/411.html";
	main[413] = "./src/_default/errors/413.html";
	main[500] = "./src/_default/errors/500.html";

	for (size_t index = start; index < end; index++) {
		std::string line = trimStart(lines.at(index));
		try {
			value_config lineData = getValue(line);
			if (is_digits(lineData.index) && lineData.values.size() == 1) {
				int index = ft_atoi(lineData.index);
				main[index] = lineData.values.at(0);
			}
		} catch (std::exception &e) {
			throw std::exception();
		}
	}

	return main;
}

location_config createLocation(std::vector<std::string> lines, size_t start, size_t end) {
	location_config location;

	location.redirect_code = 0;
	location.autoindex = true;
	location.listing = false;
	location.path = "/";
	location.root = "./www";
	location.upload_store = "./upload";
	location.redirect_url = "";
	location.client_max_body_size = -1;

	for (size_t index = start; index < end; index++) {
		std::string line = trimStart(lines.at(index));
		try {
			value_config lineData = getValue(line);
			addLocation(&location, &lineData);
		} catch (std::exception &e) {
			throw std::exception();
		}
	}

	if (location.autoindex) {
		location.index.clear();
		location.index.push_back("index.html");
		location.index.push_back("index.htm");
		location.index.push_back("default_page.html");
	}

	if (location.allowed_methods.size() < 1) {
		location.allowed_methods.push_back("GET");
		location.allowed_methods.push_back("POST");
		location.allowed_methods.push_back("PUT");
		location.allowed_methods.push_back("DELETE");
	}

	return location;
}

void printLocation(location_config loc) {
	std::cout << "Root: " << loc.root << std::endl;
	std::cout << "Path: " << loc.path << std::endl;
	std::cout << "Client max body size: " << loc.client_max_body_size << std::endl;
	std::cout << "Redirect url: " << loc.redirect_url << std::endl;
	std::cout << "Redirect code: " << loc.redirect_code << std::endl;
	std::cout << "Auto Index: " << loc.autoindex << std::endl;
	std::cout << "Listing: " << loc.listing << std::endl;

	std::cout << "CGI Extensions: " << std::endl;

	for (size_t i = 0; i < loc.cgi_extension.size(); i++) {
		std::cout << "	" << loc.cgi_extension.at(i) << std::endl;
	}

	std::cout << "Indexes: " << std::endl;

	for (size_t i = 0; i < loc.index.size(); i++) {
		std::cout << "	" << loc.index.at(i) << std::endl;
	}
	std::cout << "Methods: " << std::endl;

	for (size_t i = 0; i < loc.allowed_methods.size(); i++) {
		std::cout << "	" << loc.allowed_methods.at(i) << std::endl;
	}
}

void extractData(std::string line, std::map<std::string, std::string> *map) {
	std::string value;

	int end = 0;
	int start = skip_space(line);

	for (size_t i = (size_t)start; i < line.length(); i++) {
		if ((line[i] == ' ' || line[i] == '\t' || line[i] == '\n' || line[i] == '\r' || line[i] == '\f' || line[i] == '\v') &&
			(line[i + 1] == ' ' || line[i + 1] == '\t' || line[i + 1] == '\n' || line[i + 1] == '\r' || line[i + 1] == '\f' || line[i + 1] == '\v')) {
			end = (int)i;
			break;
		}
	}

	value = line.substr(start, end - start);
	line = line.substr(end, line.size() - end);

	for (size_t i = (size_t)start; i < line.length(); i++) {
		if (!(line[i] == ' ' || line[i] == '\t' || line[i] == '\n' || line[i] == '\r' || line[i] == '\f' || line[i] == '\v')) {
			line = line.substr(i, line.size() - (i + 1));
			break;
		}
	}
	std::vector<std::string> indexes = splitString(line);

	for (size_t i = 0; i < indexes.size(); i++) {
		(*map)[indexes.at(i)] = value;
	}
}

void setDataFromFile(std::string path, std::map<std::string, std::string> *map) {
	std::string file_path = path;

	int fd = open(file_path.c_str(), O_RDONLY);
	if (fd < 0) return;

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

/* EXTERNS */

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
		return it->second;	}
	return "unknown_status";
}

int Config::getTimeout(void) const {
	return this->_timeout;
}

std::vector<int> Config::getServerPorts(void) const {
	return this->_ports;
}

locationConfig Config::getLocationFromPath(std::string path) {
	std::string bestMatch = "";
	locationConfig bestLocation;

	if (path.at(0) != '/') {
		path = "/" + path;
	}

	for (std::map<std::string, location_config>::const_iterator it = this->_locations.begin();
		 it != this->_locations.end(); ++it) {
		const std::string& locationPath = it->first;

		if (path.find(locationPath) == 0) {
			if (locationPath.length() > bestMatch.length()) {
				bestMatch = locationPath;
				bestLocation = it->second;
			}
		}
	}

	return bestLocation;
}

std::string Config::getLocationRoot(std::string path) {
	try {
		locationConfig config = this->getLocationFromPath(path);
		return config.root;
	} catch (std::exception &e) {
		throw std::exception();
	}
}

std::vector<std::string> Config::getLocationIndex(std::string path) {
	try {
		locationConfig config = this->getLocationFromPath(path);
		return config.index;
	} catch (std::exception &e) {
		std::vector<std::string> empty;
		return empty;
	}
}

bool Config::isMethodAllowed(std::string path, std::string method) {
	try {
		locationConfig config = this->getLocationFromPath(path);
		int methodsCount = std::count(config.allowed_methods.begin(), config.allowed_methods.end(), method);
		if (methodsCount > 0)
			return true;
		return false;
	} catch (std::exception &e) {
		return true;
	}
}

bool Config::listLocation(std::string path, std::string url) {
	try {
		size_t dotPos = url.find_last_of('.');
		if (dotPos != std::string::npos)
			return false;

		locationConfig config = this->getLocationFromPath(path);
		return config.listing;

	} catch (std::exception &e) {
		return false;
	}
}

std::string Config::getServerInfo(void) const {
	std::string main = SERVER_NAME;
	main += "/";
	main += VERSION;
	return main;
}

std::string Config::getServerName() const {
	return this->_serverName;
}

bool Config::isCgiPath(std::string path) {
	try {
		locationConfig config = this->getLocationFromPath(path);

		if (config.cgi_extension.empty())
			return false;

		size_t dotPos = path.find_last_of('.');
		if (dotPos == std::string::npos)
			return false;

		std::string extension = path.substr(dotPos);

		for (size_t i = 0; i < config.cgi_extension.size(); i++) {
		if (config.cgi_extension[i] == extension)
				return true;
		}
		return false;

	} catch (std::exception &e) {
		return false;
	}
}

std::string Config::getCgiScriptPath(const std::string& path) {
	try {
		locationConfig config = this->getLocationFromPath(path);

		std::string scriptPath = config.root;
		std::string relativePath = path;

		if (path.find(config.path) == 0 && config.path != "/")
			relativePath = path.substr(config.path.length());
		if (!scriptPath.empty() && scriptPath[scriptPath.length() - 1] != '/' &&
			!relativePath.empty() && relativePath[0] != '/') {
			scriptPath += "/";
		}
		scriptPath += relativePath;

		return scriptPath;
	} catch (std::exception &e) {
		return "";
	}
}

std::string Config::getRedirectPath(const std::string& path) {
	try {
		locationConfig config = this->getLocationFromPath(path);
		return config.redirect_url;
	} catch (std::exception &e) {
		return "";
	}
}

std::vector<std::string> Config::getCgiExtensions(std::string path) {
	try {
		locationConfig config = this->getLocationFromPath(path);
		return config.cgi_extension;
	} catch (std::exception &e) {
		std::vector<std::string> empty;
		return empty;
	}
}

std::string Config::getErrorPath(int errorCode) {
	std::map<int, std::string>::iterator it = this->error_pages.find(errorCode);
	if (it != this->error_pages.end()) {
		return it->second;	}
	return "";
}

/* MAIN */

Config::Config(void) {}
Config::~Config(void) {}

void Config::_addValue(value_config *lineData) {
	if (lineData->index == "listen") {
		for (size_t i = 0; i < lineData->values.size(); i++) {
			if (is_digits(lineData->values.at(i)))
				this->_ports.push_back(ft_atoi(lineData->values.at(i)));
		}
	} else if (lineData->index == "host" && lineData->values.size() == 1) {
		std::string host = lineData->values.at(0);
		if (std::count(host.begin(), host.end(), '.') != 3)
			return;
		std::string hostClone = host;
		for (int i = 0; i != 3; i++) {
			size_t pos = hostClone.find_first_of('.');
			hostClone = hostClone.substr(0, pos - 1);
			if (!is_digits(hostClone))
				return;
		}
		this->_hostName = host;
	} else if (lineData->index == "server_name" && lineData->values.size() == 1) {
		this->_serverName = lineData->values.at(0);
	} else if (lineData->index == "timeout" && lineData->values.size() == 1) {
		if (is_digits(lineData->values.at(0)))
			this->_timeout = ft_atoi(lineData->values.at(0));
	}  else if (lineData->index == "client_max_body_size" && lineData->values.size() == 1) {
		if (is_digits(lineData->values.at(0)))
			this->_client_max_body_size = ft_atoi(lineData->values.at(0));
	}
}

void Config::_processParsing(void) {

	std::vector<std::string> locationBlock;

	for (size_t index = 1; index < (this->_configLines.size() - 2); index++) {
		std::string line = trimStart(this->_configLines.at(index));

		if (isIndexedBlock("location", line)) {
			try {
				std::vector<std::string> words = separateString(line);
				size_t end = getBlockEnd(this->_configLines, index + 1);
				location_config location = createLocation(this->_configLines, index + 1, end);

				location.path = words.at(1);
				this->_locations[location.path] = location;
				//this->_locations.push_back(location);

				index = end;
			} catch (std::exception &e) {
				this->parsingMessage = "Location is not closed at line " + static_cast<int>(index);
				throw std::exception();
			}
			continue;
		} else if (isIndexedBlock("error_pages", line)) {
			size_t end = getBlockEnd(this->_configLines, index + 1);
			this->error_pages = createErrorPages(this->_configLines, index + 1, end);
			continue;
		} else if (line == "}") {
			this->parsingMessage = "Empty closing brace at line " + static_cast<int>(index);
			throw std::exception();
		} else {
			value_config lineData = getValue(line);
			this->_addValue(&lineData);
			continue;
		}
	}
}

void Config::_readFile(int fd) {
	ssize_t bytesRead;
	char buffer[8192];

	while ((bytesRead = read(fd, buffer, sizeof(buffer) - 1)) > 0)
		this->_fileBuffer[bytesRead] = '\0';
	close(fd);

	this->_fileBuffer = static_cast<std::string>(buffer);

	int openBraces = std::count(this->_fileBuffer.begin(), this->_fileBuffer.end(), '{');
	int closeBraces = std::count(this->_fileBuffer.begin(), this->_fileBuffer.end(), '{');

	if (openBraces != closeBraces) {
		this->parsingMessage = "Error, missing braces.";
		throw std::exception();
	}

	std::string line;
	std::istringstream iss(this->_fileBuffer);
	while (std::getline(iss, line)) {
		size_t commentPos = line.find("//");
		if (commentPos != std::string::npos)
			line = line.substr(0, commentPos - 1);
		this->_configLines.push_back(line);
	}

	if (this->_configLines.at(0) != "server {") {
		this->parsingMessage = "Wrong config start.";
		throw std::exception();
	}

	if (this->_configLines.at(this->_configLines.size() - 1) != "}") {
		this->parsingMessage = "Server config is not closed.";
		throw std::exception();
	}
}

Config::Config(std::string fileName) {

	int fd = open(fileName.c_str(), O_RDONLY);
	if (fd < 0) { return; }

	this->_readFile(fd);

	this->_hostName = "127.0.0.1";
	this->_serverName = "localhost";
	this->_timeout = 60;
	this->_client_max_body_size = 1048576;

	this->error_pages[200] = "./src/_default/errors/200.html";
	this->error_pages[201] = "./src/_default/errors/201.html";
	this->error_pages[204] = "./src/_default/errors/204.html";
	this->error_pages[302] = "./src/_default/errors/302.html";
	this->error_pages[400] = "./src/_default/errors/400.html";
	this->error_pages[403] = "./src/_default/errors/403.html";
	this->error_pages[404] = "./src/_default/errors/404.html";
	this->error_pages[405] = "./src/_default/errors/405.html";
	this->error_pages[411] = "./src/_default/errors/411.html";
	this->error_pages[413] = "./src/_default/errors/413.html";
	this->error_pages[500] = "./src/_default/errors/500.html";

	try {
		this->_processParsing();
	} catch (std::exception &e) {
		throw std::exception();
	}

	if (this->_ports.size() == 0)
		this->_ports.push_back(8080);

	std::map<std::string, locationConfig>::const_iterator it;

	for (it = this->_locations.begin(); it != this->_locations.end(); ++it) {
		location_config location = it->second;
		if (location.client_max_body_size == (size_t)-1) {
			location.client_max_body_size = this->_client_max_body_size;
		}
	}

	setDataFromFile(MIME_TYPES_PATH, &this->_mimes);
	setDataFromFile(CODE_STATUS_PATH, &this->_codeStatus);

}

std::string Config::getUploadStore(const std::string& path) {
	std::cout << "CALLED" << std::endl;
	std::cout << "path: " << path << std::endl;
    try {
        locationConfig config = this->getLocationFromPath(path);
        return config.upload_store;
    } catch (std::exception &e) {
		std::cout << "exception " << path << std::endl;
        return "./upload";
    }
}