#include "../../includes/listenSocket.hpp"

ListenSocket::~ListenSocket(void) {}

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

	char tempBuffer[30000] = {0};
	ssize_t bytesRead = read(this->_newSocket, tempBuffer, 30000);
	if (bytesRead < 0) { make_error("failed to read from socket", EXIT_FAILURE); }

	tempBuffer[bytesRead] = '\0';
	this->_buffer = static_cast<std::string>(tempBuffer);

	/* DEBUG */

	{
		std::cout << "Received request from [" << BOLD << LIGHT_CYAN << sock << RESET << "], now handling request." << std::endl;
	}
}

void ListenSocket::handler(Config &server_config) {
	Request request(*this, server_config);
	Response response(request);

	this->response = response.getResponse();
	/* DEBUG */

	{
		std::string status_color;
		int status_code = request.getParsing().status_code;
		std::cout << std::endl << "Request processed, status code: ";

		if (status_code >= 100) { status_color = BLUE; }
		if (status_code >= 200) { status_color = GREEN; }
		if (status_code >= 300) { status_color = LIGHT_CYAN; }
		if (status_code >= 400) { status_color = RED; }

		std::cout << status_color << BOLD << request.getParsing().status_code << RESET;
		std::cout << ", message: " << status_color << request.getParsing().status_message << RESET << ", file_name: " << request.getFileName() << std::endl;
	}

}

void ListenSocket::responder(void) {
	write(this->_newSocket, this->response.c_str(), response.length());
	close(this->_newSocket);
	this->response = "";
}

void ListenSocket::launch(Config &server_config) {
	while (true) {
		std::cout << "----------------------------------------" << std::endl;
		this->accepter();
		this->handler(server_config);
		this->responder();
	}
}
