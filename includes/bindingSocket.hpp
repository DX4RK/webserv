#pragma once

#include "socket.hpp"

class BindingSocket : public Socket {
public:
	BindingSocket( int domain, int type, int protocol, u_long interface, Config &server_config );
	~BindingSocket( void );

	int connect_network(int sock, struct sockaddr_in adress);
private:
};
