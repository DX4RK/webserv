#include "../../includes/connectSocket.hpp"

ConnectSocket::~ConnectSocket(void) {}
ConnectSocket::ConnectSocket(int domain, int type, int protocol, u_long interface, Config *config) : Socket(domain, type, protocol, interface, config, 8080) {
	this->server_config = config;
	this->_connection = this->connect_network(this->_sock, this->get_address());
	if (this->_connection < 0) {
		std::cerr << "failed to ethablish connection." << std::endl;
		exit(EXIT_FAILURE);
	}
}

int ConnectSocket::connect_network(int sock, struct sockaddr_in adress) {
	return connect(sock, (struct sockaddr *)&adress, sizeof(adress));
}
