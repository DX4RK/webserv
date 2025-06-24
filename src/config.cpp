#include "config.hpp"

void extractData(std::string line, std::map<std::string, std::string> *map);
void setDataFromFile(std::string path, std::map<std::string, std::string> *map);

Config::Config(std::string configPath) {
	this->_ports.push_back(8080);
	this->_serverName = "localhost";
	this->_timeout = 60;

	locationConfig root_config;
	root_config.path = "/";
	root_config.root = "./www";
	root_config.index.push_back("index.html");
	root_config.autoindex = false;
	root_config.client_max_body_size = 1048576;
	root_config.allowed_methods.push_back("GET");
	root_config.allowed_methods.push_back("POST");
	root_config.allowed_methods.push_back("PUT");
	root_config.allowed_methods.push_back("DELETE");
	root_config.redirect_code = 0;

	this->_locations[root_config.path] = root_config;

	if (!configPath.empty()) {
		this->_parseConfigFile(configPath);
	}
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
		return it->second;	}
	return "unknown_status";
}

std::vector<int> Config::getServerPorts(void) const {
	return this->_ports;
}

int Config::getTimeout(void) const {
	return this->_timeout;
}

std::string Config::getLocationRoot(std::string path) {
	std::map<std::string, location_config>::const_iterator it = this->_locations.find(path);
	if (it != this->_locations.end()) {
		return it->second.root;
	}
	return "./www";
}

bool Config::isLocationMethodsAllowed(std::string path, std::string method) {
	std::cout << "----------------" << std::endl;
	std::cout << path << std::endl;
	std::map<std::string, location_config>::const_iterator it = this->_locations.find(path);
	if (it != this->_locations.end()) {
		int methodsCount = std::count(it->second.allowed_methods.begin(), it->second.allowed_methods.end(), method);
		std::cout << methodsCount << std::endl;
		if (methodsCount > 0)
			return true;
		return false;
	}
	std::cout << "no locations defined, all methods are allowed." << std::endl;
	return (true);
}

std::vector<std::string> Config::getLocationIndex(std::string path) const {
	std::map<std::string, location_config>::const_iterator it = this->_locations.find(path);
	if (it != this->_locations.end()) {
		return it->second.index;
	}
	throw std::exception();
}

std::string Config::getServerInfo() const {
	std::string main = SERVER_NAME;
	main += "/";
	main += VERSION;
	return main;
}

std::string Config::getServerName() const {
	return this->_serverName;
}

void Config::_parseConfigFile(const std::string& configPath) {
    int fd = open(configPath.c_str(), O_RDONLY);
    if (fd < 0) { return; }

    char buffer[8192];
    std::string content;
    ssize_t bytesRead;

    while ((bytesRead = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        content += buffer;
    }
    close(fd);

    size_t serverPos = content.find("server {");
    if (serverPos == std::string::npos) { return; }

    size_t start = content.find("{", serverPos) + 1;
    size_t braceCount = 1;
    size_t end = start;

    while (end < content.length() && braceCount > 0) {
        if (content[end] == '{') { braceCount++; }
        else if (content[end] == '}') { braceCount--; }
        end++;
    }

    std::string serverBlock = content.substr(start, end - start - 1);

    this->_ports.clear();
    this->_parseServerBlock(serverBlock);

    if (this->_ports.empty())
        this->_ports.push_back(8080);
}

void Config::_parseLocation(const std::string& locationLine, std::istringstream& iss) {
	size_t pathStart = locationLine.find("location ") + 9;
	size_t pathEnd = locationLine.find(" {");
	if (pathEnd == std::string::npos) { pathEnd = locationLine.find("{"); }

	std::string path = locationLine.substr(pathStart, pathEnd - pathStart);
	path.erase(0, path.find_first_not_of(" \t"));
	path.erase(path.find_last_not_of(" \t") + 1);

	locationConfig location;
	location.path = path;
	location.root = "./www";
	location.autoindex = false;
	location.client_max_body_size = 1048576;
	location.redirect_code = 0;

	std::string line;
	while (std::getline(iss, line)) {
		size_t commentPos = line.find("//");
		if (commentPos != std::string::npos) {
			line = line.substr(0, commentPos);
		}

		line.erase(0, line.find_first_not_of(" \t"));
		line.erase(line.find_last_not_of(" \t;") + 1);

		if (line.empty()) { continue; }
		if (line == "}") { break; }

		if (line.find("root ") == 0) {
			location.root = line.substr(5);
		}
		else if (line.find("index ") == 0) {
			std::string indexStr = line.substr(6);
			std::istringstream indexIss(indexStr);
			std::string index;
			location.index.clear();
			while (indexIss >> index) {
				location.index.push_back(index);
			}
		}
		else if (line.find("allowed_methods ") == 0) {
			std::string methodsStr = line.substr(16);
			std::istringstream methodsIss(methodsStr);
			std::string method;
			location.allowed_methods.clear();
			while (methodsIss >> method) {
				location.allowed_methods.push_back(method);
			}
		}
		else if (line.find("autoindex ") == 0) {
			std::string autoindexStr = line.substr(10);
			location.autoindex = (autoindexStr == "on");
			if (location.autoindex) {
				location.index.clear();
				location.index.push_back("index.html");
				location.index.push_back("index.htm");
				location.index.push_back("default_page.html");
			}
		}
		else if (line.find("client_max_body_size ") == 0) {
			location.client_max_body_size = atoll(line.substr(21).c_str());
		}
		else if (line.find("cgi_extension ") == 0) {
			std::string extensionsStr = line.substr(14);
			std::istringstream extensionsIss(extensionsStr);
			std::string extension;
			location.cgi_extension.clear();
			while (extensionsIss >> extension) {
				location.cgi_extension.push_back(extension);
			}
		}
		else if (line.find("return ") == 0) {
			std::string returnStr = line.substr(7);
			std::istringstream returnIss(returnStr);
			std::string codeStr, url;
			if (returnIss >> codeStr >> url) {
				location.redirect_code = atoi(codeStr.c_str());
				location.redirect_url = url;
			}
		}
	}

	this->_locations[path] = location;
}

void Config::_parseServerBlock(const std::string& serverBlock) {
	std::istringstream iss(serverBlock);
	std::string line;

	while (std::getline(iss, line)) {
		size_t commentPos = line.find("//");
		if (commentPos != std::string::npos) {
			line = line.substr(0, commentPos);
		}

		line.erase(0, line.find_first_not_of(" \t"));
		line.erase(line.find_last_not_of(" \t;") + 1);

		if (line.empty()) { continue; }        if (line.find("listen ") == 0) {
            std::string listenStr = line.substr(7);

            std::istringstream iss_ports(listenStr);
            std::string portToken;

            while (std::getline(iss_ports, portToken, ',')) {
                portToken.erase(0, portToken.find_first_not_of(" \t"));
                portToken.erase(portToken.find_last_not_of(" \t") + 1);

                int port = atoi(portToken.c_str());
                if (port > 0 && port <= 65535) {
                    this->_ports.push_back(port);
                }
            }        }
        else if (line.find("server_name ") == 0) {
            this->_serverName = line.substr(12);
        }
        else if (line.find("timeout ") == 0) {
            int timeout = atoi(line.substr(8).c_str());
            if (timeout > 0) {
                this->_timeout = timeout;
            }
        }
		else if (line.find("location ") == 0) {
			this->_parseLocation(line, iss);
		}
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

void extractData(std::string line, std::map<std::string, std::string> *map) {
	std::string value;

	int end = 0;
	int start = skip_space(line);

	for (size_t i = (size_t)start; i < line.length(); i++) {
		if (isspace(line[i]) && isspace(line[i + 1])) {
			end = (int)i;
			break;
		}
	}

	value = line.substr(start, end - start);
	line = line.substr(end, line.size() - end);

	for (size_t i = (size_t)start; i < line.length(); i++) {
		if (!isspace(line[i])) {
			line = line.substr(i, line.size() - (i + 1));
			break;
		}
	}
	std::vector<std::string> indexes = splitString(line);

	for (size_t i = 0; i < indexes.size(); i++) {
		(*map)[indexes.at(i)] = value;
	}
}

locationConfig* Config::findLocationForPath(const std::string& path) const {
	std::string bestMatch = "";
	locationConfig* bestLocation = NULL;

	for (std::map<std::string, location_config>::const_iterator it = this->_locations.begin();
		 it != this->_locations.end(); ++it) {
		const std::string& locationPath = it->first;

		if (path.find(locationPath) == 0) {
			if (locationPath.length() > bestMatch.length()) {
				bestMatch = locationPath;
				bestLocation = const_cast<locationConfig*>(&it->second);
			}
		}
	}

	return bestLocation;
}

bool Config::isCgiPath(const std::string& path) const {
	locationConfig* location = findLocationForPath(path);
	if (!location) {
		return false;
	}

	if (location->cgi_extension.empty()) {
		return false;
	}

	size_t dotPos = path.find_last_of('.');
	if (dotPos == std::string::npos) {
		return false;
	}

	std::string extension = path.substr(dotPos);

	for (size_t i = 0; i < location->cgi_extension.size(); i++) {
		if (location->cgi_extension[i] == extension) {
			return true;
		}
	}

	return false;
}

std::string Config::getCgiScriptPath(const std::string& path) const {
	locationConfig* location = findLocationForPath(path);
	if (!location) {
		return "";
	}

	std::string scriptPath = location->root;

	std::string relativePath = path;
	if (path.find(location->path) == 0 && location->path != "/") {
		relativePath = path.substr(location->path.length());
	}

	if (!scriptPath.empty() && scriptPath[scriptPath.length() - 1] != '/' &&
		!relativePath.empty() && relativePath[0] != '/') {
		scriptPath += "/";
	}

	scriptPath += relativePath;

	return scriptPath;
}

std::vector<std::string> Config::getCgiExtensions() const {
	// Retourner les extensions CGI du premier location qui en a
	std::map<std::string, location_config>::const_iterator it;
	for (it = this->_locations.begin(); it != this->_locations.end(); ++it) {
		if (!it->second.cgi_extension.empty()) {
			return it->second.cgi_extension;
		}
	}
	return std::vector<std::string>();  // Vecteur vide si aucun CGI configuré
}

std::string Config::getCgiPath() const {
	// Retourner le root du premier location CGI trouvé
	std::map<std::string, location_config>::const_iterator it;
	for (it = this->_locations.begin(); it != this->_locations.end(); ++it) {
		if (!it->second.cgi_extension.empty()) {
			return it->second.root;
		}
	}
	return "./www/cgi-bin";  // Fallback par défaut
}
