#include "Server.hpp"

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
