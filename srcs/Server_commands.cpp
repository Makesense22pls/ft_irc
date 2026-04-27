#include "Server.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <sys/socket.h>

std::string Server::extractCommand(std::string &buffer)
{
	size_t pos = buffer.find("\r\n");
	if (pos == std::string::npos)
		return "";
	
	std::string cmd = buffer.substr(0, pos);
	buffer.erase(0, pos + 2);
	return cmd;
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
	else if (!client->isRegistered())
		sendToClient(client->getFd(), "ERROR :You must register first (PASS, NICK, USER)");
	else
		sendToClient(client->getFd(), "421 " + client->getNickname() + " " + cmd + " :Unknown command");
}

void Server::handlePass(Client *client, const std::string &args)
{
	if (client->isRegistered())
	{
		sendToClient(client->getFd(), "462 :You may not reregister");
		return;
	}

	if (args.empty())
	{
		sendToClient(client->getFd(), "461 PASS :Not enough parameters");
		return;
	}

	if (args == _password)
	{
		client->setAuthenticated(true);
		std::cout << "Client fd " << client->getFd() << " authenticated" << std::endl;
	}
	else
	{
		sendToClient(client->getFd(), "464 :Password incorrect");
		removeClient(client->getFd());
	}
}

void Server::handleNick(Client *client, const std::string &args)
{
	if (!client->isAuthenticated())
	{
		sendToClient(client->getFd(), "ERROR :You must send PASS first");
		return;
	}

	if (args.empty())
	{
		sendToClient(client->getFd(), "431 :No nickname given");
		return;
	}

	std::string nickname = args;
	size_t spacePos = nickname.find(' ');
	if (spacePos != std::string::npos)
		nickname = nickname.substr(0, spacePos);

	if (isNicknameInUse(nickname))
	{
		sendToClient(client->getFd(), "433 * " + nickname + " :Nickname is already in use");
		return;
	}

	client->setNickname(nickname);
	std::cout << "Client fd " << client->getFd() << " set nickname: " << nickname << std::endl;

	if (!client->getUsername().empty() && !client->isRegistered())
	{
		client->setRegistered(true);
		sendToClient(client->getFd(), "001 " + nickname + " :Welcome to the IRC Network");
	}
}

void Server::handleUser(Client *client, const std::string &args)
{
	if (!client->isAuthenticated())
	{
		sendToClient(client->getFd(), "ERROR :You must send PASS first");
		return;
	}

	if (client->isRegistered())
	{
		sendToClient(client->getFd(), "462 :You may not reregister");
		return;
	}

	std::vector<std::string> tokens = splitCommand("USER " + args);
	if (tokens.size() < 5)
	{
		sendToClient(client->getFd(), "461 USER :Not enough parameters");
		return;
	}

	client->setUsername(tokens[1]);
	
	size_t colonPos = args.find(':');
	if (colonPos != std::string::npos)
		client->setRealname(args.substr(colonPos + 1));
	else
		client->setRealname(tokens[4]);

	std::cout << "Client fd " << client->getFd() << " set username: " << tokens[1] << std::endl;

	if (!client->getNickname().empty() && !client->isRegistered())
	{
		client->setRegistered(true);
		sendToClient(client->getFd(), "001 " + client->getNickname() + " :Welcome to the IRC Network");
	}
}

void Server::handlePing(Client *client, const std::string &args)
{
	if (args.empty())
		sendToClient(client->getFd(), "PONG");
	else
		sendToClient(client->getFd(), "PONG " + args);
}

void Server::handleQuit(Client *client, const std::string &args)
{
	std::string quitMsg = "Client quit";
	if (!args.empty())
	{
		size_t colonPos = args.find(':');
		if (colonPos != std::string::npos)
			quitMsg = args.substr(colonPos + 1);
		else
			quitMsg = args;
	}
	
	std::cout << "Client fd " << client->getFd() << " quit: " << quitMsg << std::endl;
	sendToClient(client->getFd(), "ERROR :Closing connection");
	removeClient(client->getFd());
}

void Server::handleJoin(Client *client, const std::string &args)
{
	if (!client->isRegistered())
	{
		sendToClient(client->getFd(), "451 :You have not registered");
		return;
	}

	if (args.empty())
	{
		sendToClient(client->getFd(), "461 JOIN :Not enough parameters");
		return;
	}

	std::string channelName = args;
	size_t spacePos = channelName.find(' ');
	if (spacePos != std::string::npos)
		channelName = channelName.substr(0, spacePos);
	size_t commaPos = channelName.find(',');
	if (commaPos != std::string::npos)
		channelName = channelName.substr(0, commaPos);

	if (channelName.empty() || channelName[0] != '#')
	{
		sendToClient(client->getFd(), "403 " + channelName + " :No such channel");
		return;
	}

	Channel *channel = getOrCreateChannel(channelName);
	if (channel->hasMember(client->getFd()))
		return;

	channel->addMember(client);
	if (channel->getMembers().size() == 1)
		channel->addOperator(client->getFd());

	std::string prefix = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost";
	sendToChannel(channel, prefix + " JOIN " + channelName, -1);

	if (channel->getTopic().empty())
		sendToClient(client->getFd(), "331 " + client->getNickname() + " " + channelName + " :No topic is set");
	else
		sendToClient(client->getFd(), "332 " + client->getNickname() + " " + channelName + " :" + channel->getTopic());

	std::string names;
	const std::map<int, Client*> &members = channel->getMembers();
	for (std::map<int, Client*>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		if (!names.empty())
			names += " ";
		if (channel->isOperator(it->first))
			names += "@";
		names += it->second->getNickname();
	}

	sendToClient(client->getFd(), "353 " + client->getNickname() + " = " + channelName + " :" + names);
	sendToClient(client->getFd(), "366 " + client->getNickname() + " " + channelName + " :End of /NAMES list");
}

void Server::handlePart(Client *client, const std::string &args)
{
	if (!client->isRegistered())
	{
		sendToClient(client->getFd(), "451 :You have not registered");
		return;
	}

	if (args.empty())
	{
		sendToClient(client->getFd(), "461 PART :Not enough parameters");
		return;
	}

	std::string channelName = args;
	std::string partReason;
	size_t spacePos = args.find(' ');
	if (spacePos != std::string::npos)
	{
		channelName = args.substr(0, spacePos);
		partReason = args.substr(spacePos + 1);
		if (!partReason.empty() && partReason[0] == ':')
			partReason.erase(0, 1);
	}

	Channel *channel = getChannel(channelName);
	if (channel == NULL)
	{
		sendToClient(client->getFd(), "403 " + channelName + " :No such channel");
		return;
	}

	if (!channel->hasMember(client->getFd()))
	{
		sendToClient(client->getFd(), "442 " + channelName + " :You're not on that channel");
		return;
	}

	std::string prefix = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost";
	std::string message = prefix + " PART " + channelName;
	if (!partReason.empty())
		message += " :" + partReason;
	sendToChannel(channel, message, -1);

	channel->removeMember(client->getFd());
	if (channel->isEmpty())
	{
		delete _channels[channelName];
		_channels.erase(channelName);
	}
}

void Server::handlePrivmsg(Client *client, const std::string &args)
{
	if (!client->isRegistered())
	{
		sendToClient(client->getFd(), "451 :You have not registered");
		return;
	}

	if (args.empty())
	{
		sendToClient(client->getFd(), "411 :No recipient given (PRIVMSG)");
		return;
	}

	size_t spacePos = args.find(' ');
	if (spacePos == std::string::npos)
	{
		sendToClient(client->getFd(), "412 :No text to send");
		return;
	}

	std::string target = args.substr(0, spacePos);
	std::string text = args.substr(spacePos + 1);
	if (!text.empty() && text[0] == ':')
		text.erase(0, 1);

	if (text.empty())
	{
		sendToClient(client->getFd(), "412 :No text to send");
		return;
	}

	std::string prefix = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost";
	if (target[0] == '#')
	{
		Channel *channel = getChannel(target);
		if (channel == NULL)
		{
			sendToClient(client->getFd(), "403 " + target + " :No such channel");
			return;
		}
		if (!channel->hasMember(client->getFd()))
		{
			sendToClient(client->getFd(), "404 " + target + " :Cannot send to channel");
			return;
		}

		sendToChannel(channel, prefix + " PRIVMSG " + target + " :" + text, client->getFd());
	}
	else
	{
		Client *targetClient = findClientByNickname(target);
		if (targetClient == NULL)
		{
			sendToClient(client->getFd(), "401 " + target + " :No such nick/channel");
			return;
		}

		sendToClient(targetClient->getFd(), prefix + " PRIVMSG " + target + " :" + text);
	}
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

	return tokens;
}

bool Server::isNicknameInUse(const std::string &nickname)
{
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == nickname)
			return true;
	}
	return false;
}

Client *Server::findClientByNickname(const std::string &nickname)
{
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == nickname)
			return it->second;
	}
	return NULL;
}

Channel *Server::getChannel(const std::string &name)
{
	std::map<std::string, Channel*>::iterator it = _channels.find(name);
	if (it == _channels.end())
		return NULL;
	return it->second;
}

Channel *Server::getOrCreateChannel(const std::string &name)
{
	Channel *channel = getChannel(name);
	if (channel != NULL)
		return channel;

	channel = new Channel(name);
	_channels[name] = channel;
	return channel;
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
