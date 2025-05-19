#pragma once

#include "utils.hpp"
#include "unistd.h"
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>

class Socket
{
public:
	Socket( int domain, int type, int protocol, int port, u_long interface );
	~Socket( void );

	int get_sock( void ) const;
	struct sockaddr_in get_address( void ) const;

	virtual int connect_network(int sock, struct sockaddr_in adress) = 0;
protected:
	int _sock;
	int _connection;
	struct sockaddr_in adress;
private:
	Socket( void );
};
