#include "config.hpp"

Config::Config(std::string configPath) {

	(void)configPath;

	this->_port = 8080;
	this->_serverName = "localhost";

	locationConfig root_config;
	root_config.path = "/";
	root_config.root = "./www";
	root_config.index.push_back("index.html");

	this->_locations[root_config.path] = root_config;
}

/* UTILS */

/* GETTERS & SETTERS */

int Config::getPort( void ) const { return this->_port; }

std::string Config::getLocationRoot( std::string path ) {
	std::map<std::string, location_config>::const_iterator it = this->_locations.find(path);
	if (it != this->_locations.end()) { return it->second.root; }
	return "./www";
}
