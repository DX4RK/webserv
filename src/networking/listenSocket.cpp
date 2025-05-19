#include "../../includes/listenSocket.hpp"

ListenSocket::~ListenSocket(void) {
	close(this->_newSocket);
}

ListenSocket::ListenSocket(BindingSocket &mainSocket) : _socket(mainSocket) {
	int sock = mainSocket.get_sock();
	if (listen(sock, 10) < 0) {
		std::cerr << "failed to listen on socket!" << std::endl;
		exit (EXIT_FAILURE);
	}
}

std::string ListenSocket::getBuffer(void) const {
	return this->_buffer;
}

void ListenSocket::accepter(void) {
	int sock = this->_socket.get_sock();
	struct sockaddr_in adress = this->_socket.get_address();
	int adress_len = sizeof(adress);

	this->_newSocket = accept(sock, (struct sockaddr *)&adress, (socklen_t*)&adress_len);
	if (this->_newSocket < 0) { make_error("failed to accept socket", EXIT_FAILURE); }

	char tempBuffer[30000];
	read(this->_newSocket, tempBuffer, 30000);

	this->_buffer = static_cast<std::string>(tempBuffer);
}

void ListenSocket::handler(void) {
	Request mainRequest(*this);
	//std::cout << this->_buffer << std::endl;
	//std::cout << this->_buffer << std::endl;
}

void ListenSocket::responder(void) {
	std::string message = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";

	write(this->_newSocket, message.c_str(), message.length());
	close(this->_newSocket);
}

void ListenSocket::launch(void) {
	while (true) {
		std::cout << "--- WAITING FOR CONNECTION ---" << std::endl;

		this->accepter();
		this->handler();
		this->responder();

	}
}
