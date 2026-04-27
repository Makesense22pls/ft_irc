#include "Server.hpp"
#include <iostream>

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
