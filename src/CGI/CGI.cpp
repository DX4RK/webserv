#include "CGI.hpp"
#include <sys/vfs.h>    // For statfs
#include <sys/statvfs.h>  // For statvfs

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
	char input_tmp[] = "/tmp/cgi_input";
	char output_tmp[] = "/tmp/cgi_output";

	int input_fd = mkstemp(input_tmp);
	int output_fd = mkstemp(output_tmp);

	if (input_fd < 0 || output_fd < 0) {
		if (input_fd >= 0) close(input_fd);
		if (output_fd >= 0) close(output_fd);
		unlink(input_tmp);
		unlink(output_tmp);
		return "{\"success\": false, \"error\": \"Temp file creation failed\"}";
	}

	std::string scriptPath = this->_env["SCRIPT_NAME"];

	// Set content length for POST requests
	if (!body.empty()) {
		this->_addEnv("CONTENT_LENGTH", ft_itoa(body.length()));
	}

	pid_t pid = fork();
	if (pid < 0) {
		close(input_fd);
		close(output_fd);
		unlink(input_tmp);
		unlink(output_tmp);
		return "{\"success\": false, \"error\": \"Fork failed\"}";
	}

	if (pid == 0) {
		int in_fd = open(input_tmp, O_RDONLY);
		int out_fd = open(output_tmp, O_WRONLY | O_TRUNC);

		if (in_fd < 0 || out_fd < 0 ||
			dup2(in_fd, STDIN_FILENO) < 0 ||
			dup2(out_fd, STDOUT_FILENO) < 0) {
			_exit(1);
		}

		close(in_fd);
		close(out_fd);
		close(input_fd);
		close(output_fd);

		this->formatEnvironment();
		char *arguments[3];
		arguments[0] = const_cast<char*>(this->_executorPath.c_str());
		arguments[1] = const_cast<char*>(scriptPath.c_str());
		arguments[2] = NULL;

		execve(this->_executorPath.c_str(), arguments, this->_envp);
		std::cerr << "execve failed: " << strerror(errno) << std::endl;
		_exit(1);
	}

	// Parent process
	close(output_fd);
	bool success = true;

	// Write body data
	if (!body.empty()) {
		const size_t CHUNK_SIZE = 65536;
		size_t total_written = 0;
		const char* data = body.c_str();
		size_t remaining = body.length();

		// Write the entire body first
		while (remaining > 0 && success) {
			size_t chunk_size = (remaining < CHUNK_SIZE) ? remaining : CHUNK_SIZE;
			ssize_t written = write(input_fd, data + total_written, chunk_size);

			if (written <= 0) {
				if (errno == EINTR) continue;
				success = false;
				break;
			}

			total_written += written;
			remaining -= written;
		}

		// Make sure all data is written before closing
		fsync(input_fd);
	}

	close(input_fd);

	if (!success) {
		kill(pid, SIGTERM);
		usleep(100000);
		kill(pid, SIGKILL);
		waitpid(pid, NULL, 0);
		unlink(input_tmp);
		unlink(output_tmp);
		return "{\"success\": false, \"error\": \"Failed to write request body\"}";
	}

	// Wait for CGI process with timeout
	int status;
	time_t start_time = time(NULL);
	const time_t timeout = 120; // Increased timeout for large files

	while (true) {
		pid_t result = waitpid(pid, &status, WNOHANG);
		if (result == pid) break;

		if (result < 0 || (time(NULL) - start_time > timeout)) {
			kill(pid, SIGTERM);
			usleep(100000);
			kill(pid, SIGKILL);
			waitpid(pid, NULL, 0);
			unlink(input_tmp);
			unlink(output_tmp);
			return "{\"success\": false, \"error\": \"CGI execution timeout\"}";
		}
		usleep(10000);
	}

	// Read CGI output
	this->_output.clear();
	int out_fd = open(output_tmp, O_RDONLY);
	if (out_fd >= 0) {
		std::string output;
		char buffer[65536];
		ssize_t bytesRead;

		while ((bytesRead = read(out_fd, buffer, sizeof(buffer) - 1)) > 0) {
			output.append(buffer, bytesRead);
		}

		close(out_fd);
		this->_output = output;
	}

	unlink(input_tmp);
	unlink(output_tmp);

	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
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
	this->_addEnv("SCRIPT_NAME", scriptPath);  // This is needed!
	this->_addEnv("PATH", "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin");
	this->_addEnv("QUERY_STRING", "");
	this->_addEnv("SERVER_PROTOCOL", this->_protocol);
	this->_addEnv("SERVER_SOFTWARE", config.getServerInfo());
	this->_addEnv("GATEWAY_INTERFACE", "CGI/1.1");
	this->_addEnv("SERVER_NAME", config.getServerName());
	this->_addEnv("SERVER_PORT", ft_itoa(this->_serverPort));

	// For POST requests, Content-Length will be set in execute()
	this->_addEnv("CONTENT_TYPE", "application/octet-stream");  // Default content type for binary data

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
