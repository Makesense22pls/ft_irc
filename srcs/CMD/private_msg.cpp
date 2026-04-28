#include "Server.hpp"

void Server::handlePrivmsg(Client *client, const std::string &args)
{
	if (!client->isRegistered())
	{
		sendToClient(client->getFd(), "451 :You have not registered");
		return;
	}
	if (args.empty())
	{
		sendToClient(client->getFd(), "411 :No recipient given (PRIVMSG)");
		return;
	}
	size_t spacePos = args.find(' ');
	if (spacePos == std::string::npos)
	{
		sendToClient(client->getFd(), "412 :No text to send");
		return;
	}
	std::string target = args.substr(0, spacePos);
	std::string text = args.substr(spacePos + 1);
	if (!text.empty() && text[0] == ':')
		text.erase(0, 1);
	if (text.empty())
	{
		sendToClient(client->getFd(), "412 :No text to send");
		return;
	}
	std::string prefix = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost";
	if (target[0] == '#')
	{
		Channel *channel = getChannel(target);
		if (channel == NULL)
		{
			sendToClient(client->getFd(), "403 " + target + " :No such channel");
			return;
		}
		if (!channel->hasMember(client->getFd()))
		{
			sendToClient(client->getFd(), "404 " + target + " :Cannot send to channel");
			return;
		}
		sendToChannel(channel, prefix + " PRIVMSG " + target + " :" + text, client->getFd());
	}
	else
	{
		Client *targetClient = findClientByNickname(target);
		if (targetClient == NULL)
		{
			sendToClient(client->getFd(), "401 " + target + " :No such nick/channel");
			return;
		}
		sendToClient(targetClient->getFd(), prefix + " PRIVMSG " + target + " :" + text);
	}
}
