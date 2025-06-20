#include "listenSocket.hpp"

ListenSocket::~ListenSocket(void) {
	this->_pollfds.clear();
	this->_clientBuffers.clear();
	this->_sockets.clear();
}

ListenSocket::ListenSocket(std::vector<BindingSocket*> bindingSockets, Config *config) : _sockets(bindingSockets) {
	this->server_config = config;

	// Setup listening pour tous les sockets
	for (size_t i = 0; i < this->_sockets.size(); i++) {
		int sock = this->_sockets[i]->get_sock();
		if (listen(sock, 10) < 0) {
			std::cerr << "failed to listen on socket!" << std::endl;
			exit (EXIT_FAILURE);
		}
	}

	std::vector<int> ports = config->getServerPorts();
	std::cout << LIGHT_BLUE << BOLD << "[webserv]" << RESET << " listening on ports: " << BOLD;
	for (size_t i = 0; i < ports.size(); i++) {
		std::cout << ports[i];
		if (i < ports.size() - 1) std::cout << ", ";
	}
	std::cout << RESET << std::endl;
}

std::string ListenSocket::getBuffer(void) const {
	return this->_buffer;
}

void ListenSocket::accepter(void) {
	for (size_t i = 0; i < this->_sockets.size(); i++) {
		int sock = this->_sockets[i]->get_sock();

		for (size_t j = 0; j < this->_pollfds.size(); j++) {
			if (this->_pollfds[j].fd == sock && (this->_pollfds[j].revents & POLLIN)) {
				struct sockaddr_in adress = this->_sockets[i]->get_address();
				int adress_len = sizeof(adress);

				this->_newSocket = accept(sock, (struct sockaddr *)&adress, (socklen_t*)&adress_len);
				if (this->_newSocket < 0)
					make_error("failed to accept socket", EXIT_FAILURE);
				return;
			}
		}
	}
}

void ListenSocket::handler() {
	char tempBuffer[4096];

	ssize_t bytesRead;
	size_t contentLength = 0;

	std::string request;
	bool headersParsed = false;

	while (true) {
		bytesRead = recv(this->_newSocket, tempBuffer, sizeof(tempBuffer) - 1, 0);
		if (bytesRead <= 0) break;

		tempBuffer[bytesRead] = '\0';
		request.append(tempBuffer, bytesRead);

		if (!headersParsed) {
			size_t headerEnd = request.find("\r\n\r\n");
			if (headerEnd != std::string::npos) {
				headersParsed = true;

				std::string headers = request.substr(0, headerEnd);
				size_t pos = headers.find("Content-Length:");
				if (pos != std::string::npos) {
					size_t start = headers.find_first_of("0123456789", pos);
					if (start != std::string::npos) {
						contentLength = std::atoi(headers.c_str() + start);
					}
				}

				size_t totalNeeded = headerEnd + 4 + contentLength;
				if (request.size() >= totalNeeded) break;
			}
		} else {
			size_t headerEnd = request.find("\r\n\r\n");
			size_t totalNeeded = headerEnd + 4 + contentLength;
			if (request.size() >= totalNeeded) break;
		}
	}

	this->_buffer = request;

	Request req(*this, server_config);
	Response response(req, server_config);

	this->response = response.getResponse();

	/* DEBUG */

	{
		std::string status_color;
		int status_code = response.getResponseCode();

		if (status_code >= 100) { status_color = BLUE; }
		if (status_code >= 200) { status_color = GREEN; }
		if (status_code >= 300) { status_color = LIGHT_CYAN; }
		if (status_code >= 400) { status_color = RED; }

		//std::cout << "Received " << BOLD << LIGHT_PURPLE << req.getMethod() << RESET << " code: " << status_code << " url: " << req.getUrl() << std::endl;
		std::cout << LIGHT_BLUE << BOLD << "[webserv]" << RESET << " treated request " << LIGHT_ORANGE << BOLD << status_code << RESET << DIM << " " << req.getUrl() << RESET << std::endl;
	}

}

void ListenSocket::responder(void) {
	write(this->_newSocket, this->response.c_str(), response.length());
	close(this->_newSocket);
	this->response = "";
}

void ListenSocket::launch(volatile sig_atomic_t &keepRunning) {
	std::vector<int> ports = this->server_config->getServerPorts();
	std::cout << LIGHT_BLUE << BOLD
	<< "[webserv]" << RESET
	<< " website is ready, urls: " << GREEN << BOLD;

	for (size_t i = 0; i < ports.size(); i++) {
		std::cout << "http://" << this->server_config->getServerName() << ":" << ports[i];
		if (i < ports.size() - 1) std::cout << ", ";
	}
	std::cout << RESET << std::endl << std::endl;

	// Ajouter sockets listening au poll
	for (size_t i = 0; i < this->_sockets.size(); i++)
	{
		pollfd listenPfd;
		listenPfd.fd = this->_sockets[i]->get_sock();
		listenPfd.events = POLLIN;
		listenPfd.revents = 0;
		this->_pollfds.push_back(listenPfd);
	}

	while (keepRunning)
	{
		int pollResult = poll(&this->_pollfds[0], this->_pollfds.size(), 1000);

		if (pollResult < 0) {
			if (errno == EINTR) // interrupted by signal
				continue;
			make_error("poll() failed", EXIT_FAILURE);
		}

		// Vérifier nouveaux clients sur sockets listening
		for (size_t i = 0; i < this->_sockets.size(); i++) {
			int listenSock = this->_sockets[i]->get_sock();

			for (size_t j = 0; j < this->_pollfds.size(); j++) {
				if (this->_pollfds[j].fd == listenSock && (this->_pollfds[j].revents & POLLIN)) {
					// Nouveau client sur socket listening
					struct sockaddr_in adress = this->_sockets[i]->get_address();
					int adress_len = sizeof(adress);

					int newSocket = accept(listenSock, (struct sockaddr *)&adress, (socklen_t*)&adress_len);
					if (newSocket >= 0) {
						// Ajouter nouveau client au poll
						pollfd clientPfd;
						clientPfd.fd = newSocket;
						clientPfd.events = POLLIN;
						clientPfd.revents = 0;
						this->_pollfds.push_back(clientPfd);
						this->_clientBuffers[newSocket] = "";
					}
					this->_pollfds[j].revents = 0;
				}
			}
		}

		// Traiter clients existants
		for (size_t i = this->_sockets.size(); i < this->_pollfds.size(); )
		{
			if (this->_pollfds[i].revents & POLLIN)
			{
				this->_newSocket = this->_pollfds[i].fd;
				this->handler();
				this->responder();

				// Supprimer client du poll
				this->_clientBuffers.erase(this->_newSocket);
				this->_pollfds.erase(this->_pollfds.begin() + i);
			}
			else
				i++;
		}
	}
}
