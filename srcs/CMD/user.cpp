#include "Server.hpp"
#include <iostream>
#include <vector>

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
