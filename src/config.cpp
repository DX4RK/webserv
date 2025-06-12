#include "config.hpp"
#include <cstdlib>
#include <stdexcept>

void extractData(std::string line, std::map<std::string, std::string> *map);
void setDataFromFile(std::string path, std::map<std::string, std::string> *map);

/*int getPortFromEnvOrDefault(int defaultPort) {
    char *portEnv = getenv("PORT");
    if (portEnv) {
        try {
            int port = std::stoi(std::string(portEnv));
            return port;
        } catch (...) {
            std::cerr << "Warning: Invalid PORT env var, using default\n";
        }
    }
    return defaultPort;
}*/

Config::Config(std::string configPath) {
	// Configuration par défaut
	this->_port = 8080;
	this->_serverName = "localhost";

	// Configuration de base pour compatibilité
	locationConfig root_config;
	root_config.path = "/";
	root_config.root = "./www";
	root_config.index.push_back("index.html");
	root_config.autoindex = false;
	root_config.client_max_body_size = 1048576;
	root_config.allowed_methods.push_back("GET");
	root_config.redirect_code = 0;

	this->_locations[root_config.path] = root_config;

	// Parser le fichier de configuration
	if (!configPath.empty() && configPath != "test") {
		parseConfigFile(configPath);
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
		return it->second;
	}
	return "???";
}

/* UTILS */

/* GETTERS & SETTERS */

int Config::getServerPort( void ) const { return this->_port; }

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

std::string Config::getServerInfo() const {
	std::string main = SERVER_NAME;
	main += "/";
	main += VERSION;
	return main;
}

std::string Config::getServerName() const { return this->_serverName; }

/* NOUVELLES MÉTHODES POUR LE PARSING DE CONFIGURATION */

void Config::parseConfigFile(const std::string& configPath) {
	int fd = open(configPath.c_str(), O_RDONLY);
	if (fd < 0) {
		std::cerr << "Warning: Cannot open config file " << configPath << ", using default config\n";
		return;
	}

	char tempBuffer[50000] = {0};
	ssize_t bytesRead = read(fd, tempBuffer, 50000);
	close(fd);
	
	if (bytesRead < 0) {
		std::cerr << "Warning: Cannot read config file " << configPath << "\n";
		return;
	}

	std::string content = std::string(tempBuffer, bytesRead);
	
	// Parser les blocs server
	size_t pos = 0;
	while ((pos = content.find("server {", pos)) != std::string::npos) {
		serverConfig server;
		server.port = 8080;
		server.host = "127.0.0.1";
		server.server_name = "localhost";
		server.client_max_body_size = 1048576;
		
		size_t blockStart = content.find("{", pos) + 1;
		size_t blockEnd = findMatchingBrace(content, pos);
		
		if (blockEnd == std::string::npos) break;
		
		std::string serverBlock = content.substr(blockStart, blockEnd - blockStart);
		parseServerBlock(serverBlock, server);
		
		this->_servers.push_back(server);
		
		// Mettre à jour la config par défaut avec le premier serveur
		if (this->_servers.size() == 1) {
			this->_port = server.port;
			this->_serverName = server.server_name;
		}
		
		pos = blockEnd + 1;
	}
}

void Config::parseServerBlock(const std::string& block, serverConfig& server) {
	std::istringstream iss(block);
	std::string line;
	
	while (std::getline(iss, line)) {
		line = trim(line);
		
		if (line.find("listen ") == 0) {
			std::string portStr = line.substr(7);
			size_t semicolon = portStr.find(';');
			if (semicolon != std::string::npos) {
				portStr = portStr.substr(0, semicolon);
				server.port = atoi(trim(portStr).c_str());
			}
		}
		else if (line.find("host ") == 0) {
			std::string hostStr = line.substr(5);
			size_t semicolon = hostStr.find(';');
			if (semicolon != std::string::npos) {
				server.host = trim(hostStr.substr(0, semicolon));
			}
		}
		else if (line.find("server_name ") == 0) {
			std::string nameStr = line.substr(12);
			size_t semicolon = nameStr.find(';');
			if (semicolon != std::string::npos) {
				server.server_name = trim(nameStr.substr(0, semicolon));
			}
		}
		else if (line.find("client_max_body_size ") == 0) {
			std::string sizeStr = line.substr(21);
			size_t semicolon = sizeStr.find(';');
			if (semicolon != std::string::npos) {
				sizeStr = sizeStr.substr(0, semicolon);
				server.client_max_body_size = atoll(trim(sizeStr).c_str());
			}
		}
	}
	
	// Parser les locations
	size_t pos = 0;
	while ((pos = block.find("location ", pos)) != std::string::npos) {
		locationConfig location;
		location.autoindex = false;
		location.client_max_body_size = server.client_max_body_size;
		location.redirect_code = 0;
		
		// Extraire le path
		size_t pathStart = pos + 9;
		size_t pathEnd = block.find(" {", pathStart);
		if (pathEnd == std::string::npos) break;
		
		location.path = trim(block.substr(pathStart, pathEnd - pathStart));
		
		// Extraire le bloc location
		size_t locationStart = block.find("{", pathEnd) + 1;
		size_t locationEnd = findMatchingBrace(block, pathEnd);
		if (locationEnd == std::string::npos) break;
		
		std::string locationBlock = block.substr(locationStart, locationEnd - locationStart);
		parseLocationBlock(locationBlock, location);
		
		server.locations[location.path] = location;
		pos = locationEnd + 1;
	}
	
	// Parser error_pages
	size_t errorPos = block.find("error_pages {");
	if (errorPos != std::string::npos) {
		size_t errorStart = block.find("{", errorPos) + 1;
		size_t errorEnd = findMatchingBrace(block, errorPos);
		if (errorEnd != std::string::npos) {
			std::string errorBlock = block.substr(errorStart, errorEnd - errorStart);
			std::istringstream errorIss(errorBlock);
			std::string errorLine;
			
			while (std::getline(errorIss, errorLine)) {
				errorLine = trim(errorLine);
				if (errorLine.empty() || errorLine[0] == '}') continue;
				
				size_t spacePos = errorLine.find(' ');
				if (spacePos != std::string::npos) {
					std::string codeStr = errorLine.substr(0, spacePos);
					std::string pagePath = errorLine.substr(spacePos + 1);
					size_t semicolon = pagePath.find(';');
					if (semicolon != std::string::npos) {
						pagePath = pagePath.substr(0, semicolon);					try {
						int code = atoi(codeStr.c_str());
						server.error_pages[code] = trim(pagePath);
					} catch (...) {}
					}
				}
			}
		}
	}
}

void Config::parseLocationBlock(const std::string& block, locationConfig& location) {
	std::istringstream iss(block);
	std::string line;
	
	while (std::getline(iss, line)) {
		line = trim(line);
		if (line.empty()) continue;
		
		if (line.find("root ") == 0) {
			std::string rootStr = line.substr(5);
			size_t semicolon = rootStr.find(';');
			if (semicolon != std::string::npos) {
				location.root = trim(rootStr.substr(0, semicolon));
			}
		}
		else if (line.find("index ") == 0) {
			std::string indexStr = line.substr(6);
			size_t semicolon = indexStr.find(';');
			if (semicolon != std::string::npos) {
				indexStr = indexStr.substr(0, semicolon);
				location.index = parseList(indexStr);
			}
		}
		else if (line.find("allowed_methods ") == 0) {
			std::string methodsStr = line.substr(16);
			size_t semicolon = methodsStr.find(';');
			if (semicolon != std::string::npos) {
				methodsStr = methodsStr.substr(0, semicolon);
				location.allowed_methods = parseList(methodsStr);
			}
		}
		else if (line.find("autoindex ") == 0) {
			std::string autoindexStr = line.substr(10);
			size_t semicolon = autoindexStr.find(';');
			if (semicolon != std::string::npos) {
				autoindexStr = trim(autoindexStr.substr(0, semicolon));
				location.autoindex = (autoindexStr == "on");
			}
		}
		else if (line.find("upload_store ") == 0) {
			std::string storeStr = line.substr(13);
			size_t semicolon = storeStr.find(';');
			if (semicolon != std::string::npos) {
				location.upload_store = trim(storeStr.substr(0, semicolon));
			}
		}
		else if (line.find("client_max_body_size ") == 0) {
			std::string sizeStr = line.substr(21);
			size_t semicolon = sizeStr.find(';');
			if (semicolon != std::string::npos) {
				sizeStr = sizeStr.substr(0, semicolon);
				location.client_max_body_size = atoll(trim(sizeStr).c_str());
			}
		}
		else if (line.find("cgi_extension ") == 0) {
			std::string extStr = line.substr(14);
			size_t semicolon = extStr.find(';');
			if (semicolon != std::string::npos) {
				extStr = extStr.substr(0, semicolon);
				location.cgi_extension = parseList(extStr);
			}
		}
		else if (line.find("cgi_path ") == 0) {
			std::string pathStr = line.substr(9);
			size_t semicolon = pathStr.find(';');
			if (semicolon != std::string::npos) {
				pathStr = pathStr.substr(0, semicolon);
				location.cgi_path = parseList(pathStr);
			}
		}
		else if (line.find("return ") == 0) {
			std::string returnStr = line.substr(7);
			size_t semicolon = returnStr.find(';');
			if (semicolon != std::string::npos) {
				returnStr = trim(returnStr.substr(0, semicolon));
				size_t spacePos = returnStr.find(' ');
				if (spacePos != std::string::npos) {
					location.redirect_code = atoi(returnStr.substr(0, spacePos).c_str());
					location.redirect_url = trim(returnStr.substr(spacePos + 1));
				}
			}
		}
	}
}

std::string Config::trim(const std::string& str) {
	size_t start = str.find_first_not_of(" \t\n\r");
	if (start == std::string::npos) return "";
	size_t end = str.find_last_not_of(" \t\n\r");
	return str.substr(start, end - start + 1);
}

size_t Config::findMatchingBrace(const std::string& content, size_t start) {
	size_t openPos = content.find("{", start);
	if (openPos == std::string::npos) return std::string::npos;
	
	int braceCount = 1;
	size_t pos = openPos + 1;
	
	while (pos < content.length() && braceCount > 0) {
		if (content[pos] == '{') braceCount++;
		else if (content[pos] == '}') braceCount--;
		pos++;
	}
	
	return braceCount == 0 ? pos - 1 : std::string::npos;
}

std::vector<std::string> Config::parseList(const std::string& line) {
	std::vector<std::string> result;
	std::istringstream iss(line);
	std::string item;
	
	while (iss >> item) {
		result.push_back(item);
	}
	
	return result;
}

/* NOUVEAUX GETTERS */

const std::vector<serverConfig>& Config::getServers() const {
	return this->_servers;
}

const serverConfig* Config::getServerByPort(int port) const {
	for (size_t i = 0; i < this->_servers.size(); i++) {
		if (this->_servers[i].port == port) {
			return &this->_servers[i];
		}
	}
	return NULL;
}

const locationConfig* Config::getLocationConfig(const std::string& path, int serverPort) const {
	const serverConfig* server = NULL;
	if (serverPort != -1) {
		server = getServerByPort(serverPort);
	} else if (!this->_servers.empty()) {
		server = &this->_servers[0];
	}
	
	if (server) {
		// Chercher la location la plus spécifique
		std::string bestMatch = "";
		const locationConfig* bestLocation = NULL;
		
		for (std::map<std::string, locationConfig>::const_iterator it = server->locations.begin(); 
			 it != server->locations.end(); ++it) {
			if (path.find(it->first) == 0 && it->first.length() > bestMatch.length()) {
				bestMatch = it->first;
				bestLocation = &it->second;
			}
		}
		
		return bestLocation;
	}
	
	return NULL;
}

std::vector<std::string> Config::getAllowedMethods(const std::string& path, int serverPort) const {
	const locationConfig* loc = getLocationConfig(path, serverPort);
	if (loc && !loc->allowed_methods.empty()) {
		return loc->allowed_methods;
	}
	std::vector<std::string> defaultMethods;
	defaultMethods.push_back("GET");
	return defaultMethods;
}

bool Config::isAutoindexEnabled(const std::string& path, int serverPort) const {
	const locationConfig* loc = getLocationConfig(path, serverPort);
	return loc ? loc->autoindex : false;
}

std::string Config::getUploadStore(const std::string& path, int serverPort) const {
	const locationConfig* loc = getLocationConfig(path, serverPort);
	return loc ? loc->upload_store : "";
}

size_t Config::getClientMaxBodySize(const std::string& path, int serverPort) const {
	const locationConfig* loc = getLocationConfig(path, serverPort);
	if (loc && loc->client_max_body_size > 0) {
		return loc->client_max_body_size;
	}
	
	const serverConfig* server = getServerByPort(serverPort);
	return server ? server->client_max_body_size : 1048576;
}

std::string Config::getErrorPage(int errorCode, int serverPort) const {
	const serverConfig* server = getServerByPort(serverPort);
	if (server) {
		std::map<int, std::string>::const_iterator it = server->error_pages.find(errorCode);
		if (it != server->error_pages.end()) {
			return it->second;
		}
	}
	return "";
}
