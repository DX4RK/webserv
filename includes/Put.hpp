#ifndef PUT_HPP
#define PUT_HPP

#include "method.hpp"
#include "request.hpp"
#include "response.hpp"
#include "config.hpp"
#include "CGI.hpp"
#include "utils.hpp"

class Response;

class Put : public Method {
	public:
		Put( void );
		Put( Request &request, Config *config );
		~Put( void );

		void process( Response &response, Request &request );

	private:
		std::string _fileName;
		std::string _filePath;
		Config *server_config;

		bool _handleFileUrl( Request &request, const std::string root );
		bool _hasWritePermission( const std::string &filePath );
};

#endif