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
	std::set<int>			_invited;
	bool					_inviteOnly;
	std::string				_key;
	bool					_topicOnlyByOp;
	int						_limit;

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

	bool isInvited(int fd) const;
	void addInvitation(int fd);
	void removeInvitation(int fd);

	bool isInviteOnly() const;
	void setInviteOnly(bool b);
	const std::string &getKey() const;
	void setKey(const std::string &key);
	bool isTopicOnlyByOp() const;
	void setTopicOnlyByOp(bool b);
	int getLimit() const;
	void setLimit(int limit);

	const std::map<int, Client*> &getMembers() const;
};

#endif