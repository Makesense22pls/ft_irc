#include "Server.hpp"

void Server::handlePing(Client *client, const std::string &args)
{
	if (args.empty())
		sendToClient(client->getFd(), "PONG");
	else
		sendToClient(client->getFd(), "PONG " + args);
}
