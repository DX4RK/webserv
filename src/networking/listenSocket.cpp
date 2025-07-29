#include "listenSocket.hpp"

ListenSocket::~ListenSocket(void) {
	this->_pollfds.clear();
	this->_clientBuffers.clear();
	this->_sockets.clear();
}

ListenSocket::ListenSocket(std::vector<BindingSocket*> bindingSockets, std::vector<Config*> configs)
	: _sockets(bindingSockets), _configs(configs) {
	// Setup listening for all sockets
	for (size_t i = 0; i < this->_sockets.size(); i++) {
		int sock = this->_sockets[i]->get_sock();
		if (listen(sock, 10) < 0) {
			std::cerr << "failed to listen on socket!" << std::endl;
			exit(EXIT_FAILURE);
		}
	}

	std::cout << LIGHT_BLUE << BOLD << "[webserv]" << RESET << " listening on ports: " << BOLD;
	for (size_t i = 0; i < this->_sockets.size(); i++) {
		if (i < this->_configs.size() && this->_configs[i]) {
			std::vector<int> ports = this->_configs[i]->getServerPorts();
			for (size_t j = 0; j < ports.size(); j++) {
				std::cout << ports[j];
				if (j < ports.size() - 1) std::cout << ", ";
			}
		}
		if (i < this->_sockets.size() - 1) std::cout << ", ";
	}
	std::cout << RESET << std::endl;
}

std::string ListenSocket::getBuffer(void) const {
	return this->_buffer;
}

void ListenSocket::accepter(void) {
	// Accept on all listening sockets, and map new client fd to its config
	for (size_t i = 0; i < this->_sockets.size(); i++) {
		int sock = this->_sockets[i]->get_sock();
		for (size_t j = 0; j < this->_pollfds.size(); j++) {
			if (this->_pollfds[j].fd == sock && (this->_pollfds[j].revents & POLLIN)) {
				struct sockaddr_in adress = this->_sockets[i]->get_address();
				int adress_len = sizeof(adress);
				this->_newSocket = accept(sock, (struct sockaddr *)&adress, (socklen_t*)&adress_len);
				if (this->_newSocket < 0)
					make_error("failed to accept socket", EXIT_FAILURE);
				// Map new client fd to config index
				this->_clientFdToConfigIdx[this->_newSocket] = i;
				return;
			}
		}
	}
// Add this to the private section of ListenSocket:
// std::map<int, size_t> _clientFdToConfigIdx;
}

void ListenSocket::handler() {
	char tempBuffer[4096];

	ssize_t bytesRead;
	size_t contentLength = 0;

	std::string request;
	bool headersParsed = false;
	bool isChunked = false;

	int errorCode = 0;
	int maxTime = 10;
	double timeCounter = 0;

	clock_t thisTime = clock();
	clock_t lastTime = thisTime;

	while (true) {
		thisTime = clock();
		timeCounter += (double)(thisTime - lastTime);
		lastTime = thisTime;

		if (timeCounter > (double)(maxTime * CLOCKS_PER_SEC)) {
			std::cout << "MAX TIME REACHED!" << std::endl;
			errorCode = 408;
			break;
		}

		bytesRead = recv(this->_newSocket, tempBuffer, sizeof(tempBuffer) - 1, 0);
		if (bytesRead <= 0)
			break;

		tempBuffer[bytesRead] = '\0';
		request.append(tempBuffer, bytesRead);

		// Step 1: Parse headers if not done yet
		if (!headersParsed) {
			size_t headerEnd = request.find("\r\n\r\n");
			if (headerEnd != std::string::npos) {
				headersParsed = true;

				std::string headers = request.substr(0, headerEnd);

				// Check for Transfer-Encoding: chunked
				if (headers.find("Transfer-Encoding: chunked") != std::string::npos) {
					isChunked = true;
				} else {
					// Check for Content-Length
					size_t pos = headers.find("Content-Length:");
					if (pos != std::string::npos) {
						size_t start = headers.find_first_of("0123456789", pos);
						if (start != std::string::npos) {
							contentLength = std::atoi(headers.c_str() + start);
						}
					}
				}
			}
		}

		// Step 2: Stop when full body is received
		if (headersParsed) {
			if (isChunked) {
				// Wait until we see the end of a chunked body: \r\n0\r\n\r\n
				if (request.find("\r\n0\r\n\r\n") != std::string::npos)
					break;
			} else {
				size_t headerEnd = request.find("\r\n\r\n");
				size_t totalNeeded = headerEnd + 4 + contentLength;
				if (request.size() >= totalNeeded)
					break;
			}
		}
	}

	this->_buffer = request;


	// Use the config mapped to this client fd
	Config* config = NULL;
	std::map<int, size_t>::iterator it = this->_clientFdToConfigIdx.find(this->_newSocket);
	if (it != this->_clientFdToConfigIdx.end() && it->second < this->_configs.size()) {
		config = this->_configs[it->second];
	} else if (!this->_configs.empty()) {
		config = this->_configs[0];
	}
	if (!config) {
		std::cerr << "[webserv] ERROR: No config found for client fd " << this->_newSocket << std::endl;
		return;
	}
	Request req(*this, config, errorCode);
	Response response(req, config);
	this->response = response.getResponse();

	/* DEBUG */

	{
		std::string status_color;
		int status_code = response.getResponseCode();

		if (status_code >= 100) { status_color = BLUE; }
		if (status_code >= 200) { status_color = GREEN; }
		if (status_code >= 300) { status_color = LIGHT_CYAN; }
		if (status_code >= 400) { status_color = RED; }

		std::cout << "Status code: " << status_code << std::endl;
		//std::cout << "Received " << BOLD << LIGHT_PURPLE << req.getMethod() << RESET << " code: " << status_code << " url: " << req.getUrl() << std::endl;
		//std::cout << LIGHT_BLUE << BOLD << "[webserv]" << RESET << " treated request " << LIGHT_ORANGE << BOLD << status_code << RESET << DIM << " " << req.getUrl() << RESET << std::endl;
	}
}

void ListenSocket::responder(void) {
	write(this->_newSocket, this->response.c_str(), response.length());
	close(this->_newSocket);
	// Remove mapping for this client fd
	this->_clientFdToConfigIdx.erase(this->_newSocket);
	this->response = "";
}

void ListenSocket::launch(volatile sig_atomic_t &keepRunning) {
	std::cout << LIGHT_BLUE << BOLD
	<< "[webserv]" << RESET
	<< " website is ready, urls: " << GREEN << BOLD;
	for (size_t i = 0; i < this->_sockets.size(); i++) {
		Config* config = (i < this->_configs.size()) ? this->_configs[i] : NULL;
		if (config) {
			std::vector<int> ports = config->getServerPorts();
			for (size_t j = 0; j < ports.size(); j++) {
				std::cout << "http://" << config->getServerName() << ":" << ports[j];
				if (j < ports.size() - 1) std::cout << ", ";
			}
			if (i < this->_sockets.size() - 1) std::cout << ", ";
		}
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
