#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include <vector>
#include <poll.h>
#include "Client.hpp"
#include "Channel.hpp"

class Server
{
private:
	int							_serverSocket;
	int							_port;
	std::string					_password;
	std::map<int, Client*>		_clients;
	std::map<std::string, Channel*>	_channels;
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
	void handleJoin(Client *client, const std::string &args);
	void handlePart(Client *client, const std::string &args);
	void handlePrivmsg(Client *client, const std::string &args);
	void handleTopic(Client *client, const std::string &args);
	void handleInvite(Client *client, const std::string &args);
	void handleKick(Client *client, const std::string &args);
	void handleMode(Client *client, const std::string &args);
	
	std::vector<std::string> splitCommand(const std::string &command);
	bool isNicknameInUse(const std::string &nickname);
	Client *findClientByNickname(const std::string &nickname);
	Channel *getChannel(const std::string &name);
	Channel *getOrCreateChannel(const std::string &name);
	void removeClientFromChannels(int fd);
	void sendToChannel(Channel *channel, const std::string &message, int exceptFd);

public:
	Server(int port, const std::string &password);
	~Server();

	void init();
	void run();
	void sendToClient(int fd, const std::string &message);
};

#endif
