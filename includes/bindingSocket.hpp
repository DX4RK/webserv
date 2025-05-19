#pragma once

#include "socket.hpp"

class BindingSocket : public Socket {
public:
	BindingSocket( int domain, int type, int protocol, int port, u_long interface );
	~BindingSocket( void );

	int connect_network(int sock, struct sockaddr_in adress);
private:
};
