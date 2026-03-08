#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include <vector>
#include <poll.h>
#include "Client.hpp"

class Server
{
private:
	int							_serverSocket;
	int							_port;
	std::string					_password;
	std::map<int, Client*>		_clients;
	std::vector<struct pollfd>	_pollfds;
	bool						_running;

	void acceptNewClient();
	void handleClientData(int fd);
	void removeClient(int fd);
	void setNonBlocking(int fd);
	std::string extractCommand(std::string &buffer);
	void processCommand(Client *client, const std::string &command);

public:
	Server(int port, const std::string &password);
	~Server();

	void init();
	void run();
	void sendToClient(int fd, const std::string &message);
};

#endif
