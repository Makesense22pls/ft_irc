#include "Server.hpp"
#include <iostream>

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
