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
	char input_tmp[] = "/tmp/cgi_input_XXXXXX";
	char output_tmp[] = "/tmp/cgi_output_XXXXXX";
	int input_fd = mkstemp(input_tmp);
	int output_fd = mkstemp(output_tmp);
	if (input_fd < 0 || output_fd < 0) {
		if (input_fd >= 0) close(input_fd);
		if (output_fd >= 0) close(output_fd);
		return "{\"success\": false, \"error\": \"Temp file creation failed\"}";
	}

	std::string scriptPath = this->_env["SCRIPT_NAME"];
	std::cout << "Executing CGI script: " << scriptPath << std::endl;
	std::cout << "Executing CGI Path: " << this->_executorPath << std::endl;
	//if (this->_executorPath.empty() || access(scriptPath.c_str(), F_OK | X_OK) != 0) {
	//	close(input_fd); close(output_fd);
	//	unlink(input_tmp); unlink(output_tmp);
	//	return "{\"success\": false, \"error\": \"Invalid script or executor\"}";
	//}

	struct timeval tv;
	tv.tv_sec = 30;
	tv.tv_usec = 0;

	pid_t pid = fork();
	if (pid < 0) {
		close(input_fd); close(output_fd);
		unlink(input_tmp); unlink(output_tmp);
		return "{\"success\": false, \"error\": \"Fork failed\"}";
	}

	if (pid == 0) {
		// Child process
		// close(input_fd); // Close parent's input fd
		// close(output_fd); // Close parent's output fd

		int in_fd = open(input_tmp, O_RDONLY);
		int out_fd = open(output_tmp, O_WRONLY | O_TRUNC);
		if (in_fd < 0 || out_fd < 0) {
			exit(1);
		}
		if (dup2(in_fd, STDIN_FILENO) < 0 || dup2(out_fd, STDOUT_FILENO) < 0) {
			close(in_fd); close(out_fd);
			exit(1);
		}
		close(in_fd); close(out_fd);

		this->formatEnvironment();
		char *arguments[3];
		arguments[0] = const_cast<char*>(this->_executorPath.c_str());
		arguments[1] = const_cast<char*>(scriptPath.c_str());
		arguments[2] = NULL;

		execve(this->_executorPath.c_str(), arguments, this->_envp);
		exit(1);
	}

	std::string result;
	bool success = true;

	// Write body to temp input file
	if (!body.empty()) {
		size_t total_written = 0;
		const char* data = body.c_str();
		size_t remaining = body.length();
		while (remaining > 0) {
			ssize_t written = write(input_fd, data + total_written, remaining < 4096 ? remaining : 4096);
			if (written < 0) {
				success = false;
				break;
			}
			total_written += written;
			remaining -= written;
		}
	}
	close(input_fd);

	// Wait for child and read output from temp file
	int status;
	struct timeval wait_tv = tv;
	fd_set waitfds;
	FD_ZERO(&waitfds);

	while (true) {
		pid_t result = waitpid(pid, &status, WNOHANG);
		if (result == pid) break;
		if (result < 0 || select(0, &waitfds, NULL, NULL, &wait_tv) <= 0) {
			kill(pid, SIGTERM);
			usleep(100000);
			kill(pid, SIGKILL);
			waitpid(pid, NULL, 0);
			success = false;
			break;
		}
		usleep(1000);
	}

	this->_output.clear();
	int out_fd = open(output_tmp, O_RDONLY);
	if (out_fd >= 0 && success) {
		char buffer[4096];
		ssize_t bytesRead;
		while ((bytesRead = read(out_fd, buffer, sizeof(buffer) - 1)) > 0) {
			buffer[bytesRead] = '\0';
			this->_output += buffer;
		}
		close(out_fd);
	} else {
		success = false;
	}
	unlink(input_tmp);
	unlink(output_tmp);

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
	(void)scriptPath;
	this->_addEnv("REQUEST_METHOD", ft_upper(this->_method));
	//this->_addEnv("SCRIPT_NAME", scriptPath);
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
