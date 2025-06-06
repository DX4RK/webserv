#pragma once

#include "_libs.hpp"

#include "utils.hpp"

#define VERSION "1.0"
#define SERVER_NAME "webserv"

#define MIME_TYPES_PATH "./config/mandatory/mime.types"
#define CODE_STATUS_PATH "./config/mandatory/code.status"

#define WEB_ROOT "./www"
#define NOT_FOUND_PAGE "./src/error_pages/404.html"

struct cgi_config {
	int timeout;
	std::string cgi_bin_path;
	std::vector<std::string> allowed_extensions;
};

typedef struct location_config {
	std::string path;
	std::string root;
	std::vector<std::string> index;
} locationConfig;

class Config {
public:
	Config( std::string configPath );

	int getServerPort( void ) const;
	std::string getServerInfo( void ) const ;
	std::string getServerName( void ) const ;
	std::string getLocationRoot( std::string path );
	std::string getStatusCode(const std::string& code);
	std::string getContentType(const std::string& fileName);
private:
	int _port;
	std::string _serverName;

	cgi_config _cgiConfig;
	std::map<std::string, std::string> _mimes;
	std::map<std::string, std::string> _codeStatus;
	std::map<std::string, location_config> _locations;

	Config();
};
