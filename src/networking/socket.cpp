#include "socket.hpp"

Socket::Socket(void) {}

Socket::Socket(int domain, int type, int protocol, u_long interface, Config *config, int port) {
	this->server_config = config;

	adress.sin_family = domain;
	adress.sin_port = htons(port);
	adress.sin_addr.s_addr = htonl(interface);

	this->_sock = socket(domain, type, protocol);

	if (this->_sock < 0) {
		std::cerr << "failed to create socket for port " << port << "." << std::endl;
		throw std::runtime_error("socket creation failed");
	}

	int option = 1;
	if (setsockopt(this->_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0) {
		std::cerr << "failed to set socket options for port " << port << "." << std::endl;
		throw std::runtime_error("setsockopt failed");
	}
}

Socket::~Socket(void) {}

int Socket::get_sock(void) const { return (this->_sock); }
struct sockaddr_in Socket::get_address(void) const { return (this->adress); }
