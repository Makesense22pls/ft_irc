#include "Server.hpp"
#include <vector>
#include <map>
#include <sys/socket.h>

std::string Server::extractCommand(std::string &buffer)
{
	size_t pos = buffer.find("\r\n");
	if (pos == std::string::npos)
		return ("");
	
	std::string cmd = buffer.substr(0, pos);
	buffer.erase(0, pos + 2);
	return (cmd);
}

void Server::processCommand(Client *client, const std::string &command)
{
	if (command.empty())
		return;

	std::vector<std::string> tokens = splitCommand(command);
	if (tokens.empty())
		return;

	std::string cmd = tokens[0];
	std::string args = "";
	if (tokens.size() > 1)
	{
		size_t pos = command.find(' ');
		if (pos != std::string::npos)
			args = command.substr(pos + 1);
	}

	if (cmd == "PASS")
		handlePass(client, args);
	else if (cmd == "NICK")
		handleNick(client, args);
	else if (cmd == "USER")
		handleUser(client, args);
	else if (cmd == "PING")
		handlePing(client, args);
	else if (cmd == "QUIT")
		handleQuit(client, args);
	else if (cmd == "JOIN")
		handleJoin(client, args);
	else if (cmd == "PART")
		handlePart(client, args);
	else if (cmd == "PRIVMSG")
		handlePrivmsg(client, args);
	else if (cmd == "TOPIC")
		handleTopic(client, args);
	else if (cmd == "INVITE")
		handleInvite(client, args);
	else if (!client->isRegistered())
		sendToClient(client->getFd(), "ERROR :You must register first (PASS, NICK, USER)");
	else
		sendToClient(client->getFd(), "421 " + client->getNickname() + " " + cmd + " :Unknown command");
}

std::vector<std::string> Server::splitCommand(const std::string &command)
{
	std::vector<std::string> tokens;
	std::string token;
	bool inTrailing = false;

	for (size_t i = 0; i < command.length(); ++i)
	{
		if (command[i] == ':' && !inTrailing && (i == 0 || command[i - 1] == ' '))
		{
			inTrailing = true;
			if (!token.empty())
			{
				tokens.push_back(token);
				token.clear();
			}
			continue;
		}

		if (inTrailing)
		{
			token += command[i];
		}
		else if (command[i] == ' ')
		{
			if (!token.empty())
			{
				tokens.push_back(token);
				token.clear();
			}
		}
		else
		{
			token += command[i];
		}
	}
	if (!token.empty())
		tokens.push_back(token);
	return (tokens);
}

bool Server::isNicknameInUse(const std::string &nickname)
{
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == nickname)
			return (true);
	}
	return (false);
}

Client *Server::findClientByNickname(const std::string &nickname)
{
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == nickname)
			return (it->second);
	}
	return (NULL);
}

Channel *Server::getChannel(const std::string &name)
{
	std::map<std::string, Channel*>::iterator it = _channels.find(name);
	if (it == _channels.end())
		return (NULL);
	return (it->second);
}

Channel *Server::getOrCreateChannel(const std::string &name)
{
	Channel *channel = getChannel(name);
	if (channel != NULL)
		return (channel);
	channel = new Channel(name);
	_channels[name] = channel;
	return (channel);
}

void Server::sendToChannel(Channel *channel, const std::string &message, int exceptFd)
{
	const std::map<int, Client*> &members = channel->getMembers();
	for (std::map<int, Client*>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		if (it->first == exceptFd)
			continue;
		sendToClient(it->first, message);
	}
}

void Server::sendToClient(int fd, const std::string &message)
{
	std::string msg = message;
	if (msg.find("\r\n") == std::string::npos)
		msg += "\r\n";
	send(fd, msg.c_str(), msg.length(), 0);
}
