#include "Server.hpp"
#include <iostream>

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
