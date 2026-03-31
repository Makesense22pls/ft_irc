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
	
	void handlePass(Client *client, const std::string &args);
	void handleNick(Client *client, const std::string &args);
	void handleUser(Client *client, const std::string &args);
	void handlePing(Client *client, const std::string &args);
	void handleQuit(Client *client, const std::string &args);
	
	std::vector<std::string> splitCommand(const std::string &command);
	bool isNicknameInUse(const std::string &nickname);

public:
	Server(int port, const std::string &password);
	~Server();

	void init();
	void run();
	void sendToClient(int fd, const std::string &message);
};

#endif
