#include "Server.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cerrno>

Server::Server(int port, const std::string &password) 
	: _serverSocket(-1), _port(port), _password(password), _running(false)
{
}

Server::~Server()
{
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		delete it->second;
	_clients.clear();
	if (_serverSocket != -1)
		close(_serverSocket);
}

void Server::init()
{
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket < 0)
		throw std::runtime_error("Failed to create socket");

	int opt = 1;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("Failed to set socket options");

	setNonBlocking(_serverSocket);

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(_port);

	if (bind(_serverSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		throw std::runtime_error("Failed to bind socket");

	if (listen(_serverSocket, 10) < 0)
		throw std::runtime_error("Failed to listen on socket");

	struct pollfd serverPoll;
	serverPoll.fd = _serverSocket;
	serverPoll.events = POLLIN;
	serverPoll.revents = 0;
	_pollfds.push_back(serverPoll);

	std::cout << "Server initialized and listening on port " << _port << std::endl;
}

void Server::run()
{
	_running = true;
	
	while (_running)
	{
		int pollCount = poll(&_pollfds[0], _pollfds.size(), -1);
		
		if (pollCount < 0)
		{
			if (errno == EINTR)
				continue;
			throw std::runtime_error("Poll failed");
		}

		for (size_t i = 0; i < _pollfds.size(); ++i)
		{
			if (_pollfds[i].revents & POLLIN)
			{
				if (_pollfds[i].fd == _serverSocket)
					acceptNewClient();
				else
					handleClientData(_pollfds[i].fd);
			}
		}
	}
}

void Server::acceptNewClient()
{
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	
	int clientFd = accept(_serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
	if (clientFd < 0)
	{
		std::cerr << "Failed to accept client" << std::endl;
		return;
	}

	setNonBlocking(clientFd);

	Client *newClient = new Client(clientFd);
	_clients[clientFd] = newClient;

	struct pollfd clientPoll;
	clientPoll.fd = clientFd;
	clientPoll.events = POLLIN;
	clientPoll.revents = 0;
	_pollfds.push_back(clientPoll);

	std::cout << "New client connected: fd " << clientFd << std::endl;
}

void Server::handleClientData(int fd)
{
	char buffer[512];
	int bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);

	if (bytesRead <= 0)
	{
		if (bytesRead == 0)
			std::cout << "Client disconnected: fd " << fd << std::endl;
		else
			std::cerr << "Error reading from client: fd " << fd << std::endl;
		removeClient(fd);
		return;
	}

	buffer[bytesRead] = '\0';
	
	Client *client = _clients[fd];
	client->appendBuffer(std::string(buffer));

	std::string clientBuffer = client->getBuffer();
	std::string cmd;
	while (!(cmd = extractCommand(clientBuffer)).empty())
	{
		std::cout << "Received command from fd " << fd << ": " << cmd << std::endl;
		processCommand(client, cmd);
	}
	client->clearBuffer();
	client->appendBuffer(clientBuffer);
}

void Server::removeClient(int fd)
{
	close(fd);
	
	delete _clients[fd];
	_clients.erase(fd);

	for (std::vector<struct pollfd>::iterator it = _pollfds.begin(); it != _pollfds.end(); ++it)
	{
		if (it->fd == fd)
		{
			_pollfds.erase(it);
			break;
		}
	}
}

void Server::setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		throw std::runtime_error("Failed to get socket flags");
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
		throw std::runtime_error("Failed to set non-blocking mode");
}

std::string Server::extractCommand(std::string &buffer)
{
	size_t pos = buffer.find("\r\n");
	if (pos == std::string::npos)
		return "";
	
	std::string cmd = buffer.substr(0, pos);
	buffer.erase(0, pos + 2);
	return cmd;
}

void Server::processCommand(Client *client, const std::string &command)
{
	// Implementer command processing pass,nick,user, ping quit
	(void)client;
	(void)command;
}

void Server::sendToClient(int fd, const std::string &message)
{
	std::string msg = message;
	if (msg.find("\r\n") == std::string::npos)
		msg += "\r\n";
	
	send(fd, msg.c_str(), msg.length(), 0);
}
