#pragma once

#include "_libs.hpp"

#include "request.hpp"
#include "bindingSocket.hpp"

#define MAX_CLIENTS 100
#define BUFFER_SIZE 30000

class ListenSocket {
public:
	~ListenSocket( void );
	ListenSocket( std::vector<BindingSocket*> bindingSockets, Config *config );

	void launch( void );
	std::string getBuffer( void ) const;
private:
	int _newSocket;

	std::string _buffer;
	std::string response;

	std::vector<pollfd> _pollfds;
	std::map<int, std::string> _clientBuffers;

	std::vector<BindingSocket*> _sockets;
	Config *server_config;

	void accepter( void );
	void handler( void );
	void responder( void );

	ListenSocket( void );
};
