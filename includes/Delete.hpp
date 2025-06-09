#pragma once

#include "method.hpp"
#include "request.hpp"
#include "response.hpp"
#include "config.hpp"
#include "CGI.hpp"
#include "utils.hpp"

class Response;

class Delete : public Method {
	public:
		Delete( void );
		Delete( Request &request, Config *config );
		~Delete( void );

		void process( Response &response, Request &request );

	private:
		std::string _fileName;
		std::string _filePath;
		Config *server_config;

		bool _handleFileUrl( Request &request, const std::string root );
		void _handleStandardLogin(Request &request);
		void _handleForumPost(Request &request);
		std::string _getSessionUser(Request &request);
};
