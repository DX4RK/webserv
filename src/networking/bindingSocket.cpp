#include "bindingSocket.hpp"

BindingSocket::~BindingSocket(void) { if (this->_sock >= 0) { close(this->_sock); } }

BindingSocket::BindingSocket(int domain, int type, int protocol, u_long interface, Config *config, int port) : Socket(domain, type, protocol, interface, config, port) {
	int opt = 1;
	this->server_config = config;

	if (setsockopt(this->_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		std::cerr << "Error setting SO_REUSEADDR for port " << port << "." << std::endl;
		exit(EXIT_FAILURE);
	}

	this->_connection = this->connect_network(this->_sock, this->get_address());
	if (this->_connection < 0) {
		std::cerr << "failed to establish connection on port " << port << "." << std::endl;
		exit(EXIT_FAILURE);
	}
}

int BindingSocket::connect_network(int sock, struct sockaddr_in adress) {
	return bind(sock, (struct sockaddr *)&adress, sizeof(adress));
}
