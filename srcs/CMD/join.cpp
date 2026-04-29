#include "Server.hpp"
#include <map>

void Server::handleJoin(Client *client, const std::string &args)
{
	if (!client->isRegistered())
	{
		sendToClient(client->getFd(), "451 :You have not registered");
		return;
	}

	if (args.empty())
	{
		sendToClient(client->getFd(), "461 JOIN :Not enough parameters");
		return;
	}

	std::string channelName = args;
	size_t spacePos = channelName.find(' ');
	if (spacePos != std::string::npos)
		channelName = channelName.substr(0, spacePos);
	size_t commaPos = channelName.find(',');
	if (commaPos != std::string::npos)
		channelName = channelName.substr(0, commaPos);

	if (channelName.empty() || channelName[0] != '#')
	{
		sendToClient(client->getFd(), "403 " + channelName + " :No such channel");
		return;
	}

	Channel *channel = getOrCreateChannel(channelName);
	if (channel->hasMember(client->getFd()))
		return;

	
	if (channel->isInviteOnly() && !channel->isInvited(client->getFd()))
	{
		sendToClient(client->getFd(), "473 " + channelName + " :Cannot join channel (invite only)");
		return;
	}

	
	if (!channel->getKey().empty())
	{
		size_t spacePos = args.find(' ');
		std::string key;
		if (spacePos != std::string::npos)
			key = args.substr(spacePos + 1);
		if (key != channel->getKey())
		{
			sendToClient(client->getFd(), "475 " + channelName + " :Cannot join channel (incorrect key)");
			return;
		}
	}
	if (channel->getLimit() > 0 && (int)channel->getMembers().size() >= channel->getLimit())
	{
		sendToClient(client->getFd(), "471 " + channelName + " :Cannot join channel (channel is full)");
		return;
	}

	channel->addMember(client);
	channel->removeInvitation(client->getFd());
	if (channel->getMembers().size() == 1)
		channel->addOperator(client->getFd());
	std::string prefix = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost";
	sendToChannel(channel, prefix + " JOIN " + channelName, -1);
	if (channel->getTopic().empty())
		sendToClient(client->getFd(), "331 " + client->getNickname() + " " + channelName + " :No topic is set");
	else
		sendToClient(client->getFd(), "332 " + client->getNickname() + " " + channelName + " :" + channel->getTopic());
	std::string names;
	const std::map<int, Client*> &members = channel->getMembers();
	for (std::map<int, Client*>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		if (!names.empty())
			names += " ";
		if (channel->isOperator(it->first))
			names += "@";
		names += it->second->getNickname();
	}
	sendToClient(client->getFd(), "353 " + client->getNickname() + " = " + channelName + " :" + names);
	sendToClient(client->getFd(), "366 " + client->getNickname() + " " + channelName + " :End of /NAMES list");
}
