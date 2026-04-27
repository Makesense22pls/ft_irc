#include "Server.hpp"

void Server::handleInvite(Client *client, const std::string &args)
{
	if (!client->isRegistered())
	{
		sendToClient(client->getFd(), "451 :You have not registered");
		return;
	}

	if (args.empty())
	{
		sendToClient(client->getFd(), "461 INVITE :Not enough parameters");
		return;
	}

	size_t spacePos = args.find(' ');
	if (spacePos == std::string::npos)
	{
		sendToClient(client->getFd(), "461 INVITE :Not enough parameters");
		return;
	}

	std::string nick = args.substr(0, spacePos);
	std::string channelName = args.substr(spacePos + 1);
	size_t nextSpacePos = channelName.find(' ');
	if (nextSpacePos != std::string::npos)
		channelName = channelName.substr(0, nextSpacePos);

	Client *target = findClientByNickname(nick);
	if (target == NULL)
	{
		sendToClient(client->getFd(), "401 " + nick + " :No such nick/channel");
		return;
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

	if (!channel->isOperator(client->getFd()))
	{
		sendToClient(client->getFd(), "482 " + channelName + " :You're not channel operator");
		return;
	}

	if (channel->hasMember(target->getFd()))
	{
		sendToClient(client->getFd(), "443 " + nick + " " + channelName + " :is already on channel");
		return;
	}

	channel->addInvitation(target->getFd());
	sendToClient(client->getFd(), "341 " + client->getNickname() + " " + nick + " " + channelName);
	sendToClient(target->getFd(), ":" + client->getNickname() + "!" + client->getUsername() + "@localhost INVITE " + nick + " :" + channelName);
}
