#include <cstdlib>
#include "Server.hpp"

void Server::handleMode(Client *client, const std::string &args)
{
	if (!client->isRegistered())
	{
		sendToClient(client->getFd(), "451 :You have not registered");
		return;
	}
	if (args.empty())
	{
		sendToClient(client->getFd(), "461 MODE :Not enough parameters");
		return;
	}

	size_t sep = args.find(' ');
	std::string chanName = args.substr(0, sep);
	std::string modeStr = (sep != std::string::npos) ? args.substr(sep + 1) : "";

	Channel *chan = getChannel(chanName);
	if (!chan)
	{
		sendToClient(client->getFd(), "403 " + chanName + " :No such channel");
		return;
	}
	if (!chan->hasMember(client->getFd()))
	{
		sendToClient(client->getFd(), "442 " + chanName + " :You're not on that channel");
		return;
	}
	if (!chan->isOperator(client->getFd()))
	{
		sendToClient(client->getFd(), "482 " + chanName + " :You're not channel operator");
		return;
	}

	
	if (modeStr.empty())
	{
		std::string modes;
		if (chan->isInviteOnly()) modes += "i";
		if (!chan->getKey().empty()) modes += "k";
		if (chan->isTopicOnlyByOp()) modes += "t";
		if (chan->getLimit() > 0) modes += "l";
		sendToClient(client->getFd(), "324 " + client->getNickname() + " " + chanName + " +" + modes);
		return;
	}

	std::string prefix = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost";
	bool add = true;
	size_t argPos = modeStr.find(' ');
	std::string arg = (argPos != std::string::npos) ? modeStr.substr(argPos + 1) : "";
	std::string modes = (argPos != std::string::npos) ? modeStr.substr(0, argPos) : modeStr;

	for (size_t i = 0; i < modes.size(); ++i)
	{
		char c = modes[i];
		if (c == '+') { add = true; continue; }
		if (c == '-') { add = false; continue; }

		std::string modeMsg = (add ? "+" : "-");
		modeMsg += c;

		if (c == 'i')
		{
			chan->setInviteOnly(add);
			sendToChannel(chan, prefix + " MODE " + chanName + " " + modeMsg, -1);
		}
		else if (c == 'k')
		{
			if (add && arg.empty())
			{
				sendToClient(client->getFd(), "461 MODE :Not enough parameters");
				return;
			}
			chan->setKey(add ? arg : "");
			if (add) modeMsg += " " + arg;
			sendToChannel(chan, prefix + " MODE " + chanName + " " + modeMsg, -1);
		}
		else if (c == 't')
		{
			chan->setTopicOnlyByOp(add);
			sendToChannel(chan, prefix + " MODE " + chanName + " " + modeMsg, -1);
		}
		else if (c == 'o')
		{
			if (arg.empty())
			{
				sendToClient(client->getFd(), "461 MODE :Not enough parameters");
				return;
			}
			Client *tgt = findClientByNickname(arg);
			if (!tgt)
			{
				sendToClient(client->getFd(), "401 " + arg + " :No such nick/channel");
				return;
			}
			if (!chan->hasMember(tgt->getFd()))
			{
				sendToClient(client->getFd(), "441 " + arg + " " + chanName + " :They aren't on that channel");
				return;
			}
			if (add) chan->addOperator(tgt->getFd());
			else chan->removeOperator(tgt->getFd());
			modeMsg += " " + arg;
			sendToChannel(chan, prefix + " MODE " + chanName + " " + modeMsg, -1);
		}
		else if (c == 'l')
		{
			if (add && arg.empty())
			{
				sendToClient(client->getFd(), "461 MODE :Not enough parameters");
				return;
			}
			chan->setLimit(add ? atoi(arg.c_str()) : 0);
			if (add) modeMsg += " " + arg;
			sendToChannel(chan, prefix + " MODE " + chanName + " " + modeMsg, -1);
		}
	}
}
