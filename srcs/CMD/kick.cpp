#include "Server.hpp"

void Server::handleKick(Client *client, const std::string &args)
{
	if (!client->isRegistered())
	{
		sendToClient(client->getFd(), "451 :You have not registered");
		return;
	}

	if (args.empty())
	{
		sendToClient(client->getFd(), "461 KICK :Not enough parameters");
		return;
	}

	size_t firstSpace = args.find(' ');
	if (firstSpace == std::string::npos)
	{
		sendToClient(client->getFd(), "461 KICK :Not enough parameters");
		return;
	}

	std::string channelName = args.substr(0, firstSpace);
	std::string rest = args.substr(firstSpace + 1);
	size_t secondSpace = rest.find(' ');

	std::string targetNick = rest;
	std::string reason = client->getNickname();
	if (secondSpace != std::string::npos)
	{
		targetNick = rest.substr(0, secondSpace);
		reason = rest.substr(secondSpace + 1);
		if (!reason.empty() && reason[0] == ':')
			reason.erase(0, 1);
		if (reason.empty())
			reason = client->getNickname();
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

	Client *target = findClientByNickname(targetNick);
	if (target == NULL)
	{
		sendToClient(client->getFd(), "401 " + targetNick + " :No such nick/channel");
		return;
	}

	if (!channel->hasMember(target->getFd()))
	{
		sendToClient(client->getFd(), "441 " + targetNick + " " + channelName + " :They aren't on that channel");
		return;
	}

	std::string prefix = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost";
	sendToChannel(channel, prefix + " KICK " + channelName + " " + targetNick + " :" + reason, -1);

	channel->removeMember(target->getFd());
	if (channel->isEmpty())
	{
		delete _channels[channelName];
		_channels.erase(channelName);
	}
}
