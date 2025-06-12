#pragma once

#include "_libs.hpp"

#include "utils.hpp"

#define VERSION "1.0"
#define SERVER_NAME "webserv"

#define MIME_TYPES_PATH "./config/mandatory/mime.types"
#define CODE_STATUS_PATH "./config/mandatory/code.status"

#define WEB_ROOT "./www"

struct cgi_config {
	int timeout;
	std::string cgi_bin_path;
	std::vector<std::string> allowed_extensions;
	std::vector<std::string> cgi_paths;
};

typedef struct location_config {
	std::string path;
	std::string root;
	std::vector<std::string> index;
	std::vector<std::string> allowed_methods;
	bool autoindex;
	std::string upload_store;
	size_t client_max_body_size;
	std::vector<std::string> cgi_extension;
	std::vector<std::string> cgi_path;
	std::string redirect_url;
	int redirect_code;
} locationConfig;

typedef struct server_config {
	int port;
	std::string host;
	std::string server_name;
	size_t client_max_body_size;
	std::map<std::string, locationConfig> locations;
	std::map<int, std::string> error_pages;
} serverConfig;

class Config {
public:
	Config( std::string configPath );

	int getServerPort( void ) const;
	std::string getServerInfo( void ) const ;
	std::string getServerName( void ) const ;
	std::string getLocationRoot( std::string path );
	std::string getStatusCode(const std::string& code);
	std::string getContentType(const std::string& fileName);
	
	// Nouveaux getters pour la configuration dynamique
	const std::vector<serverConfig>& getServers() const;
	const serverConfig* getServerByPort(int port) const;
	const locationConfig* getLocationConfig(const std::string& path, int serverPort = -1) const;
	std::vector<std::string> getAllowedMethods(const std::string& path, int serverPort = -1) const;
	bool isAutoindexEnabled(const std::string& path, int serverPort = -1) const;
	std::string getUploadStore(const std::string& path, int serverPort = -1) const;
	size_t getClientMaxBodySize(const std::string& path, int serverPort = -1) const;
	std::string getErrorPage(int errorCode, int serverPort = -1) const;

private:
	// Configuration par défaut (pour compatibilité)
	int _port;
	std::string _serverName;

	// Nouvelle structure pour les serveurs multiples
	std::vector<serverConfig> _servers;
	
	cgi_config _cgiConfig;
	std::map<std::string, std::string> _mimes;
	std::map<std::string, std::string> _codeStatus;
	std::map<std::string, location_config> _locations;

	// Méthodes de parsing
	void parseConfigFile(const std::string& configPath);
	void parseServerBlock(const std::string& block, serverConfig& server);
	void parseLocationBlock(const std::string& block, locationConfig& location);
	std::string trim(const std::string& str);
	size_t findMatchingBrace(const std::string& content, size_t start);
	std::vector<std::string> parseList(const std::string& line);

	Config();
};
