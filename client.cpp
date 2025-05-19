#include <arpa/inet.h>
#include "includes/connectSocket.hpp"

int main(void) {
	ConnectSocket test_socket(AF_INET, SOCK_STREAM, 0, 8080, 0);

	struct sockaddr_in adress;
	memset(&adress, 0, sizeof(adress));

	if (inet_pton(AF_INET, "127.0.0.1", &adress.sin_addr) <= 0) {
		std::cerr << "invalid adress" << std::endl;
		exit (EXIT_FAILURE);
	}

	int socket = test_socket.get_sock();
	long main_value;
	const char *msg = "GET /index HTTP/1.1";

	char buffer[1024] = {0};

	send(socket , msg , strlen(msg) , 0);
	std::cout << "MESSAGE SENT" << std::endl;
	main_value = read(socket, buffer, 1024);
	std::cout << buffer << std::endl;

}
