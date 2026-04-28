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
	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		delete it->second;
	_channels.clear();
	if (_serverSocket != -1)
		close(_serverSocket);
}

void Server::init()
{
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0); // we setup socket server
	if (_serverSocket < 0)
		throw std::runtime_error("Failed to create server socket");
	int opt = 1;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) // options in case i stop and relaunch, so the port will be free
		throw std::runtime_error("Failed to set socket options");
	setNonBlocking(_serverSocket); // treat many clieants witouth freeze if client 1 send nothing
	struct sockaddr_in addr;  // struct whi got the adrees and the port where the serv gonna listen
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(_port);

	if (bind(_serverSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0) //th socket now listen on this adress and port
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
	std::map<int, Client*>::iterator it = _clients.find(fd);
	if (it == _clients.end())
		return;
	Client *client = it->second;
	client->appendBuffer(std::string(buffer));
	std::string clientBuffer = client->getBuffer();
	std::string cmd;
	while (!(cmd = extractCommand(clientBuffer)).empty())
	{
		std::cout << "Received command from fd " << fd << ": " << cmd << std::endl;
		processCommand(client, cmd);
		if (_clients.find(fd) == _clients.end())
			return;
		client = _clients[fd];
	}
	if (_clients.find(fd) == _clients.end())
		return;
	client->clearBuffer();
	client->appendBuffer(clientBuffer);
}

void Server::removeClient(int fd)
{
	removeClientFromChannels(fd);
	close(fd);
	std::map<int, Client*>::iterator clientIt = _clients.find(fd);
	if (clientIt != _clients.end())
	{
		delete clientIt->second;
		_clients.erase(clientIt);
	}
	for (std::vector<struct pollfd>::iterator it = _pollfds.begin(); it != _pollfds.end(); ++it)
	{
		if (it->fd == fd)
		{
			_pollfds.erase(it);
			break;
		}
	}
}

void Server::removeClientFromChannels(int fd)
{
	std::vector<std::string> emptyChannels;

	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		Channel *channel = it->second;
		if (channel->hasMember(fd))
			channel->removeMember(fd);
		if (channel->isEmpty())
			emptyChannels.push_back(it->first);
	}
	for (size_t i = 0; i < emptyChannels.size(); ++i)
	{
		delete _channels[emptyChannels[i]];
		_channels.erase(emptyChannels[i]);
	}
}

void Server::setNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("Failed to set non-blocking mode");
}
