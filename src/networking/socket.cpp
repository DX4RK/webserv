#include "socket.hpp"

Socket::Socket(void) {}
Socket::Socket(int domain, int type, int protocol, u_long interface, Config *config) {
	this->server_config = config;

	adress.sin_family = domain;
	adress.sin_port = htons(this->server_config->getServerPort());
	adress.sin_addr.s_addr = htonl(interface);

	this->_sock = socket(domain, type, protocol);

	if (this->_sock < 0) {
		std::cerr << "failed to create sointerfacecket." << std::endl;
		exit(EXIT_FAILURE);
	}
}

Socket::~Socket(void) {}

int Socket::get_sock(void) const { return (this->_sock); }
struct sockaddr_in Socket::get_address(void) const { return (this->adress); }
