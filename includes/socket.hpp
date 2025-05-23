#pragma once

#include <cstring>
#include <cstdlib>
#include <iostream>

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "utils.hpp"
#include "config.hpp"

class Socket
{
public:
	Socket( int domain, int type, int protocol, u_long interface, Config &server_config );
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
