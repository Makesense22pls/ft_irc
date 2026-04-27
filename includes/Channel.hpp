#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <map>
#include <set>
#include "Client.hpp"

class Channel
{
private:
	std::string				_name;
	std::string				_topic;
	std::map<int, Client*>	_members;
	std::set<int>			_operators;

public:
	Channel(const std::string &name);
	~Channel();

	const std::string &getName() const;
	const std::string &getTopic() const;
	void setTopic(const std::string &topic);

	bool hasMember(int fd) const;
	void addMember(Client *client);
	void removeMember(int fd);
	bool isEmpty() const;

	bool isOperator(int fd) const;
	void addOperator(int fd);
	void removeOperator(int fd);

	const std::map<int, Client*> &getMembers() const;
};

#endif