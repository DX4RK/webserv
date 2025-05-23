#include "config.hpp"
#include "bindingSocket.hpp"
#include "listenSocket.hpp"

int main(void) {

	initMimes();

	Config config("test");
	BindingSocket main_socket(AF_INET, SOCK_STREAM, 0, INADDR_ANY, config);
	ListenSocket listener(main_socket);

	listener.launch(config);

	return 0;
}
