#pragma once

#include "_libs.hpp"

#include "request.hpp"
#include "bindingSocket.hpp"

#define MAX_CLIENTS 100
#define BUFFER_SIZE 131072

class ListenSocket {
public:
	~ListenSocket( void );
	ListenSocket(std::vector<BindingSocket*> bindingSockets, std::vector<Config*> configs);

	void launch(volatile sig_atomic_t &keepRunning);
	int getNewSocket(void) const { return this->_newSocket; }
	std::string getBuffer(void) const;
private:
	int _timeout;
	int _newSocket;

	std::string _buffer;
	std::string response;

	std::vector<pollfd> _pollfds;
	std::map<int, std::string> _clientBuffers;

	std::vector<BindingSocket*> _sockets;
	std::vector<Config*> _configs; // now: one config per socket

	// Map client fd to config index
	std::map<int, size_t> _clientFdToConfigIdx;

	void accepter(void);
	void handler(void);
	void responder(void);

	ListenSocket(void);
};
