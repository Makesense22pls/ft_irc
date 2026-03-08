#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <climits>

bool isValidPort(const std::string &portStr, int &port)
{
	char *endptr;
	long val = std::strtol(portStr.c_str(), &endptr, 10);
	
	if (*endptr != '\0' || endptr == portStr.c_str())
		return false;
	if (val < 1 || val > 65535)
		return false;
	port = static_cast<int>(val);
	return true;
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}

	int port;
	if (!isValidPort(argv[1], port))
	{
		std::cerr << "Error: Invalid port (must be 1-65535)" << std::endl;
		return 1;
	}

	std::string password = argv[2];
	if (password.empty())
	{
		std::cerr << "Error: Password cannot be empty" << std::endl;
		return 1;
	}

	std::cout << "Starting IRC Server on port " << port << std::endl;
	
	return 0;
}
