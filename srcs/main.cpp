#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <csignal>
#include "Server.hpp"

Server *g_server = NULL;

void signalHandler(int signum)
{
	std::cout << "\nInterrupt signal (" << signum << ") received. Stopping server..." << std::endl;
	if (g_server)
		g_server->stop();
}

bool isValidPort(const std::string &portStr, int &port)
{
	char *endptr;
	long value = std::strtol(portStr.c_str(), &endptr, 10);
	
	if (*endptr != '\0' || endptr == portStr.c_str())
		return false;
	if (value < 1 || value > 65535)
		return false;
	port = static_cast<int>(value);
	return (true);
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return (1);
	}
	int port;
	if (!isValidPort(argv[1], port))
	{
		std::cerr << "Error: Invalid port (must be 1-65535)" << std::endl;
		return (1);
	}
	std::string password = argv[2];
	if (password.empty())
	{
		std::cerr << "Error: Password cannot be empty" << std::endl;
		return (1);
	}
	std::cout << "Starting IRC Server on port " << port << std::endl;
	
	signal(SIGINT, signalHandler);
	signal(SIGQUIT, signalHandler);
	signal(SIGPIPE, SIG_IGN);
	
	try
	{
		Server server(port, password);
		g_server = &server;
		server.init();
		server.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return (1);
	}
	
	std::cout << "Server closed safely." << std::endl;
	return (0);
}
