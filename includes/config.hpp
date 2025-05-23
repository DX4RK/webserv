#pragma once

#include <map>
#include <vector>
#include <iostream>

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

	int getPort( void ) const;
	std::string getLocationRoot( std::string path );
private:
	int _port;
	std::string _serverName;

	cgi_config _cgiConfig;
	std::map<std::string, location_config> _locations;

	Config();
};
