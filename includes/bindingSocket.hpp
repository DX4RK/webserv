#pragma once

#include "_libs.hpp"

#include "socket.hpp"

class BindingSocket : public Socket {
public:
	BindingSocket( int domain, int type, int protocol, u_long interface, Config *config, int port );
	~BindingSocket( void );

	int connect_network(int sock, struct sockaddr_in adress);
private:
};
