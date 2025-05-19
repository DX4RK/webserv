#pragma once

#include <poll.h>
#include <vector>
#include <map>
#include "request.hpp"
#include <algorithm>
#include "bindingSocket.hpp"

#define MAX_CLIENTS 100
#define BUFFER_SIZE 30000

class ListenSocket {
public:
	~ListenSocket( void );
	ListenSocket( BindingSocket &mainSocket );

	void launch( void );
	std::string getBuffer( void ) const;
private:
	ListenSocket( void );

	int _newSocket;
	std::string _buffer;
	//char _buffer[30000];

	void accepter( void );
	void handler( void );
	void responder( void );
	//void remove_socket(int clientSock);

	std::vector<pollfd> _pollfds;
	std::map<int, std::string> _clientBuffers;

	BindingSocket _socket;
};
