#include "Channel.hpp"

Channel::Channel(const std::string &name) : _name(name)
{
}

Channel::~Channel()
{
}

const std::string &Channel::getName() const
{
	return (_name);
}

const std::string &Channel::getTopic() const
{
	return (_topic);
}

void Channel::setTopic(const std::string &topic)
{
	_topic = topic;
}

bool Channel::hasMember(int fd) const
{
	return (_members.find(fd) != _members.end());
}

void Channel::addMember(Client *client)
{
	_members[client->getFd()] = client;
}

void Channel::removeMember(int fd)
{
	_members.erase(fd);
	_operators.erase(fd);
	_invited.erase(fd);
}

bool Channel::isEmpty() const
{
	return (_members.empty());
}

bool Channel::isOperator(int fd) const
{
	return (_operators.find(fd) != _operators.end());
}

void Channel::addOperator(int fd)
{
	_operators.insert(fd);
}

void Channel::removeOperator(int fd)
{
	_operators.erase(fd);
}

bool Channel::isInvited(int fd) const
{
	return _invited.find(fd) != _invited.end();
}

void Channel::addInvitation(int fd)
{
	_invited.insert(fd);
}

void Channel::removeInvitation(int fd)
{
	_invited.erase(fd);
}

const std::map<int, Client*> &Channel::getMembers() const
{
	return (_members);
}