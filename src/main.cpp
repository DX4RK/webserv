#include "bindingSocket.hpp"
#include "listenSocket.hpp"

int main(void) {

	initMimes();
	BindingSocket main_socket(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY);
	ListenSocket listener(main_socket);
	listener.launch();

	return 0;
}
