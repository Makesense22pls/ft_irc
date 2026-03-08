#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
private:
	int			_fd;
	std::string	_nickname;
	std::string	_username;
	std::string	_realname;
	std::string	_buffer;
	bool		_authenticated;
	bool		_registered;

public:
	Client(int fd);
	~Client();

	int getFd() const;
	std::string getNickname() const;
	std::string getUsername() const;
	std::string getRealname() const;
	std::string getBuffer() const;
	bool isAuthenticated() const;
	bool isRegistered() const;

	void setNickname(const std::string &nickname);
	void setUsername(const std::string &username);
	void setRealname(const std::string &realname);
	void setAuthenticated(bool auth);
	void setRegistered(bool reg);

	void appendBuffer(const std::string &data);
	void clearBuffer();
};

#endif
