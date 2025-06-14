#pragma once

#include "_libs.hpp"
#include "utils.hpp"

#define VERSION "1.0"
#define SERVER_NAME "webserv"
#define MIME_TYPES_PATH "./config/mandatory/mime.types"
#define CODE_STATUS_PATH "./config/mandatory/code.status"
#define WEB_ROOT "./www"

typedef struct location_config {
	std::string path;
	std::string root;
	std::vector<std::string> index;
	std::vector<std::string> allowed_methods;
	bool autoindex;
	std::string upload_store;
	size_t client_max_body_size;
	std::vector<std::string> cgi_extension;
	std::string redirect_url;
	int redirect_code;
} locationConfig;

class Config {
public:
	Config(std::string configPath);

	// Getters de base
	std::vector<int> getServerPorts(void) const;
	int getTimeout(void) const;
	std::string getServerInfo(void) const;
	std::string getServerName(void) const;
	std::string getLocationRoot(std::string path);
	std::string getStatusCode(const std::string& code);
	std::string getContentType(const std::string& fileName);
	
	// CGI methods (dynamiques basées sur la configuration)
	bool isCgiPath(const std::string& path) const;
	std::string getCgiScriptPath(const std::string& path) const;
	locationConfig* findLocationForPath(const std::string& path) const;

private:
	std::vector<int> _ports;
	std::string _serverName;
	int _timeout;
	std::map<std::string, std::string> _mimes;
	std::map<std::string, std::string> _codeStatus;
	std::map<std::string, location_config> _locations;
	void _parseConfigFile(const std::string& configPath);
	void _parseServerBlock(const std::string& serverBlock);
	void _parseLocation(const std::string& locationLine, std::istringstream& iss);
	std::string trim(const std::string& str);
	Config();
};
