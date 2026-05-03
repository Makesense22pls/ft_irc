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
		std::string nick = client->getNickname();
		std::string user = client->getUsername();
		std::string host = "127.0.0.1";
		sendToClient(client->getFd(), "001 " + nick + " :Welcome to the Internet Relay Network " + nick + "!" + user + "@" + host);
		sendToClient(client->getFd(), "002 " + nick + " :Your host is ircserv, running version 1.0");
		sendToClient(client->getFd(), "003 " + nick + " :This server was created today");
		sendToClient(client->getFd(), "004 " + nick + " ircserv 1.0 - itkol");
	}
}
