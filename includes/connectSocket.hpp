#pragma once

#include "socket.hpp"

class ConnectSocket : public Socket {
public:
	ConnectSocket( int domain, int type, int protocol, u_long interface, Config *config );
	~ConnectSocket( void );

	int connect_network(int sock, struct sockaddr_in adress);
private:
};
