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

void Server::sendToClient(int fd, const std::string &message)
{
	std::string msg = message;
	if (msg.find("\r\n") == std::string::npos)
		msg += "\r\n";
	
	send(fd, msg.c_str(), msg.length(), 0);
}
