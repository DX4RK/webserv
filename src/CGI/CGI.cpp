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
	char input_tmp[] = "/tmp/cgi_input_XXXXXX";
	char output_tmp[] = "/tmp/cgi_output_XXXXXX";
	std::cerr << "Creating temporary files..." << std::endl;
	int input_fd = mkstemp(input_tmp);
	int output_fd = mkstemp(output_tmp);
	if (input_fd < 0 || output_fd < 0) {
		std::cerr << "Temp file creation failed: errno=" << errno << " (" << strerror(errno) << ")" << std::endl;
		if (input_fd >= 0) {
			std::cerr << "Input file created at: " << input_tmp << std::endl;
			close(input_fd);
		}
		if (output_fd >= 0) {
			std::cerr << "Output file created at: " << output_tmp << std::endl;
			close(output_fd);
		}
		return "{\"success\": false, \"error\": \"Temp file creation failed\"}";
	}
	std::cerr << "Successfully created temp files:" << std::endl;
	std::cerr << "Input: " << input_tmp << " (fd: " << input_fd << ")" << std::endl;
	std::cerr << "Output: " << output_tmp << " (fd: " << output_fd << ")" << std::endl;

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

	// Write body to temp input file with progress checks
	if (!body.empty()) {
		// First, ensure we have enough disk space
		struct statfs fs_stat;
		if (statfs("/tmp", &fs_stat) == 0) {
			unsigned long long free_space = fs_stat.f_bsize * fs_stat.f_bavail;
			if (free_space < (unsigned long long)body.length()) {
				close(input_fd);
				unlink(input_tmp);
				unlink(output_tmp);
				return "{\"success\": false, \"error\": \"Not enough disk space\"}";
			}
		}

		// Set the exact file size we need
		if (ftruncate(input_fd, body.length()) < 0) {
			close(input_fd);
			unlink(input_tmp);
			unlink(output_tmp);
			return "{\"success\": false, \"error\": \"Failed to allocate file space\"}";
		}

		size_t total_written = 0;
		const char* data = body.c_str();
		size_t remaining = body.length();

		while (remaining > 0 && success) {
			// Write in smaller chunks and verify child process is still alive
			size_t chunk_size = (remaining < 65536) ? remaining : 65536; // 64KB chunks
			std::cerr << "Attempting to write " << chunk_size << " bytes at offset " << total_written << std::endl;
			ssize_t written = write(input_fd, data + total_written, chunk_size);

			if (written < 0) {
				if (errno == EINTR) continue;
				std::cerr << "CGI body write error: errno=" << errno << " (" << strerror(errno) << ")" << std::endl;
				std::cerr << "Failed at offset " << total_written << "/" << body.length() << std::endl;
				std::cerr << "Input file descriptor: " << input_fd << std::endl;
				success = false;
				break;
			}
			std::cerr << "Successfully wrote " << written << " bytes" << std::endl;

			total_written += written;
			remaining -= written;

			// Check if child is still alive
			if (waitpid(pid, NULL, WNOHANG) == pid) {
				success = false;
				break;
			}

			// Small delay to prevent overwhelming the system
			if (remaining > 0) {
				usleep(1000); // 1ms pause between large chunks
			}
		}

		if (!success) {
			close(input_fd);
			unlink(input_tmp);
			unlink(output_tmp);
			return "{\"success\": false, \"error\": \"Failed to write request body\"}";
		}
	}

	// Ensure all data is written to disk
	fsync(input_fd);
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
