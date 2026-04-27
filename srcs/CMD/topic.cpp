#include "Server.hpp"

void Server::handleTopic(Client *client, const std::string &args)
{
	if (!client->isRegistered())
	{
		sendToClient(client->getFd(), "451 :You have not registered");
		return;
	}

	if (args.empty())
	{
		sendToClient(client->getFd(), "461 TOPIC :Not enough parameters");
		return;
	}

	std::string channelName = args;
	std::string topicValue;
	bool hasNewTopic = false;

	size_t spacePos = args.find(' ');
	if (spacePos != std::string::npos)
	{
		channelName = args.substr(0, spacePos);
		topicValue = args.substr(spacePos + 1);
		hasNewTopic = true;
		if (!topicValue.empty() && topicValue[0] == ':')
			topicValue.erase(0, 1);
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

	if (!hasNewTopic)
	{
		if (channel->getTopic().empty())
			sendToClient(client->getFd(), "331 " + client->getNickname() + " " + channelName + " :No topic is set");
		else
			sendToClient(client->getFd(), "332 " + client->getNickname() + " " + channelName + " :" + channel->getTopic());
		return;
	}

	channel->setTopic(topicValue);
	std::string prefix = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost";
	sendToChannel(channel, prefix + " TOPIC " + channelName + " :" + topicValue, -1);
}
