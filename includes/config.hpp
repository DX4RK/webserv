#pragma once

#define WEB_ROOT "./www/"

#define VERSION "1.0"
#define SERVER_NAME "webserv"
#define MIME_TYPES_PATH "./config/mandatory/mime.types"
#define CODE_STATUS_PATH "./config/mandatory/code.status"

#define DEFAULT_CGI_SCRIPTS "./src/_default/cgi_scripts"
#define LISTING_CGI "./src/_default/cgi_scripts/listing.py"

#include "utils.hpp"

typedef struct value_config {
	std::string index;
	std::vector<std::string> values;
} value_config;

typedef struct location_config {
	int redirect_code;

	bool autoindex;
	bool listing;

	std::string path;
	std::string root;

	std::string upload_store;
	size_t client_max_body_size;
	std::string redirect_url;

	std::vector<std::string> cgi_extension;
	std::vector<std::string> index;
	std::vector<std::string> allowed_methods;
} locationConfig;

class Config {
public:
	Config( std::string fileName );
	~Config( void );

	std::string parsingMessage;

	int getTimeout(void) const;
	int getReturnCode(std::string path);

	bool listLocation(std::string path, std::string url);
	bool isCgiPath(std::string path);
	bool isMethodAllowed(std::string path, std::string method);

	std::string getLocationRoot(std::string path);
	locationConfig getLocationFromPath(std::string path);

	std::string getServerName(void) const;
	std::string getServerInfo(void) const;
	std::string getContentType(const std::string& fileName);
	std::string getStatusCode(const std::string& code);
	std::string getCgiScriptPath(const std::string& path);
	std::string getRedirectPath(const std::string& path);
	std::string getErrorPath(int errorCode);

	std::vector<int> getServerPorts(void) const;
	std::vector<std::string> getCgiExtensions(std::string path);
	std::vector<std::string> getLocationIndex(std::string path);
private:

	std::string _fileBuffer;

	int _serverTimeout;

	int _timeout;
	size_t _client_max_body_size;
	std::string _hostName;
	std::string _serverName;

	std::vector<int> _ports;

	std::vector<std::string> _configLines;
	std::map<int, std::string> error_pages;

	std::map<std::string, std::string> _mimes;
	std::map<std::string, std::string> _codeStatus;

	std::map<std::string, locationConfig> _locations;

	Config( void );

	void _readFile(int fd);
	void _processParsing(void);
	void _addValue(value_config *lineData);
};
