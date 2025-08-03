#include "CGI.hpp"

CGI::CGI(std::string method, std::string protocol, std::map<std::string, std::string> headers, int serverPort) {
	this->_envpFormatted = false;
	this->_output = "";

	this->_method = method;
	this->_protocol = protocol;
	this->_headers = headers;
	this->_serverPort = serverPort;
}

CGI::~CGI(void) {
	if (this->_envpFormatted && this->_envp) {
		for (size_t i = 0; this->_envp[i]; ++i)
			delete[] this->_envp[i];
		delete[] this->_envp;
	}
}

std::string CGI::execute(const std::string& body) {
	int stdin_pipe[2] = {-1, -1}, stdout_pipe[2] = {-1, -1};
	if (pipe(stdin_pipe) < 0 || pipe(stdout_pipe) < 0) {
		return "{\"success\": false, \"error\": \"Pipe creation failed\"}";
	}

	std::string scriptPath = this->_env["SCRIPT_NAME"];
	if (this->_executorPath.empty() || access(scriptPath.c_str(), F_OK | X_OK) != 0) {
		close(stdin_pipe[0]); close(stdin_pipe[1]);
		close(stdout_pipe[0]); close(stdout_pipe[1]);
		return "{\"success\": false, \"error\": \"Invalid script or executor\"}";
	}

	struct timeval tv;
	tv.tv_sec = 30;
	tv.tv_usec = 0;

	pid_t pid = fork();
	if (pid < 0) {
		close(stdin_pipe[0]); close(stdin_pipe[1]);
		close(stdout_pipe[0]); close(stdout_pipe[1]);
		return "{\"success\": false, \"error\": \"Fork failed\"}";
	}

	if (pid == 0) {
		close(stdin_pipe[1]);
		close(stdout_pipe[0]);

		if (dup2(stdin_pipe[0], STDIN_FILENO) < 0 ||
			dup2(stdout_pipe[1], STDOUT_FILENO) < 0) {
			_exit(1);
		}

		close(stdin_pipe[0]);
		close(stdout_pipe[1]);

		this->formatEnvironment();
		char *arguments[3];
		arguments[0] = const_cast<char*>(this->_executorPath.c_str());
		arguments[1] = const_cast<char*>(scriptPath.c_str());
		arguments[2] = NULL;

		execve(this->_executorPath.c_str(), arguments, this->_envp);
		_exit(1);
	}

	close(stdin_pipe[0]);
	close(stdout_pipe[1]);

	std::string result;
	bool success = true;

	if (!body.empty()) {
		size_t total_written = 0;
		const char* data = body.c_str();
		size_t remaining = body.length();

		while (remaining > 0) {
			fd_set writefds;
			FD_ZERO(&writefds);
			FD_SET(stdin_pipe[1], &writefds);

			struct timeval write_tv = tv;
			int ready = select(stdin_pipe[1] + 1, NULL, &writefds, NULL, &write_tv);

			if (ready <= 0) {
				success = false;
				break;
			}

			ssize_t written = write(stdin_pipe[1], data + total_written,
								  remaining < 4096 ? remaining : 4096);
			if (written <= 0) {
				success = false;
				break;
			}

			total_written += written;
			remaining -= written;
		}
	}

	close(stdin_pipe[1]);

	if (success) {
		char buffer[4096];
		this->_output.clear();

		while (true) {
			fd_set readfds;
			FD_ZERO(&readfds);
			FD_SET(stdout_pipe[0], &readfds);

			struct timeval read_tv = tv;
			int ready = select(stdout_pipe[0] + 1, &readfds, NULL, NULL, &read_tv);

			if (ready <= 0) {
				success = false;
				break;
			}

			ssize_t bytesRead = read(stdout_pipe[0], buffer, sizeof(buffer) - 1);
			if (bytesRead <= 0) break;

			buffer[bytesRead] = '\0';
			this->_output += buffer;
		}
	}

	close(stdout_pipe[0]);

	int status;
	struct timeval wait_tv = tv;
	fd_set waitfds;
	FD_ZERO(&waitfds);

	while (true) {
		pid_t result = waitpid(pid, &status, WNOHANG);
		if (result == pid) break;

		if (result < 0 ||
			select(0, &waitfds, NULL, NULL, &wait_tv) <= 0) {
			kill(pid, SIGTERM);
			usleep(100000);
			kill(pid, SIGKILL);
			waitpid(pid, NULL, 0);
			success = false;
			break;
		}
		usleep(1000);
	}

	if (!success || (WIFEXITED(status) && WEXITSTATUS(status) != 0)) {
		return "{\"success\": false, \"error\": \"CGI execution failed\"}";
	}

	return this->_output;
}

std::string CGI::getOutput() const {
	return this->_output;
}

char **CGI::formatEnvironment() {
	if (this->_envpFormatted) { return this->_envp; }

	size_t envSize = this->_env.size();
	char **envp = new char*[envSize + 1];

	size_t index = 0;
	std::map<std::string, std::string>::iterator it;
	for (it = this->_env.begin(); it != this->_env.end(); ++it, ++index) {
		std::string data = it->first + "=" + it->second;
		envp[index] = ft_strdup(data.c_str());
	}
	envp[envSize] = NULL;
	this->_envp = envp;
	this->_envpFormatted = true;
	return envp;
}

void CGI::setEnvironment( std::string scriptPath, std::string executorPath, std::string location, Config &config ) {
	(void)location;
	this->_addEnv("REQUEST_METHOD", ft_upper(this->_method));
	this->_addEnv("SCRIPT_NAME", scriptPath);
	this->_addEnv("PATH", "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin");
	this->_addEnv("QUERY_STRING", "");
	this->_addEnv("SERVER_PROTOCOL", this->_protocol);
	this->_addEnv("SERVER_SOFTWARE", config.getServerInfo());
	this->_addEnv("GATEWAY_INTERFACE", "CGI/1.1");
	this->_addEnv("SERVER_NAME", config.getServerName());
	this->_addEnv("SERVER_PORT", ft_itoa(this->_serverPort));

	this->_addEnvHeader("CONTENT_TYPE", "Content-Type");
	this->_addEnvHeader("CONTENT_LENGTH", "Content-Length");

	this->_executorPath = executorPath;
}

bool CGI::_addEnvHeader(std::string index, std::string headerIndex) {
	try {
		std::string headerValue = this->_findHeader(headerIndex);
		this->_addEnv(index, headerValue);
	} catch (std::exception &e) { return false; }
	return true;
}

void CGI::_addEnv(std::string index, std::string value) {
	this->_env[index] = value;
}

std::string CGI::_findHeader( std::string index ) {
	std::map<std::string, std::string>::const_iterator it = this->_headers.find(index);

	if (it != this->_headers.end()) return (it->second);
	throw std::exception();
}
