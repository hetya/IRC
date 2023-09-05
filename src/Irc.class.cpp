#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <poll.h>
#include <fcntl.h>
#include <vector>
#include <fstream>
#include "Irc.class.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Irc::Irc(char const *port, char const *password) : _port(std::atoi(port)), _password(password), _hostname(initialize_hostname()),_start_time(time(NULL)) {
	_channels.reserve(MAX_CHANNEL);
	setCommands(commands);
	setUnregisteredCommands(unregisteredCommands);
}


/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Irc::~Irc(void)
{
}

/*
** --------------------------------- OVERLOAD ---------------------------------
*/



/*
** --------------------------------- METHODS ----------------------------------
*/
void					Irc::sendFatalError(std::string reason )
{
	for ( std::vector< Client >::iterator pair = _clients.begin() ; pair != _clients.end() ; pair++ )
    {
		if (pair->is_register())
       		pair->sendMessageToClient("ERROR :" + reason );
    }
}
void					Irc::sendMessageToClients(std::string message )
{
	for ( std::vector< Client >::iterator pair = _clients.begin() ; pair != _clients.end() ; pair++ )
    {
		if (pair->is_register())
        	pair->sendMessageToClient( message );
    }
}

bool Irc::isChannel( std::string name )
{
	return findContainer(_channels.begin(), _channels.end(), name) != _channels.end();
}

Channel &Irc::getChannel( std::string name ) {
	return *findContainer(_channels.begin(), _channels.end(), name);
}

Channel & Irc::addChannel( std::string name )
{
	_channels.push_back( Channel( name ) );
	return ( _channels.back() );
}

void	Irc::sendClientsName( Client &client )
{
    // test status channel flags;
    for ( std::vector< Client >::iterator pair = _clients.begin() ; pair != _clients.end() ; pair++ )
    {
		if ( pair->is_register() )
        	client.sendMessageToClient( ":" + client.get_nickname() + " " + "Server" +  " :" + pair->get_nickname() );
    }
    client.sendMessageToClient( ":" + client.get_nickname() + " " + "Server"+  " :End of /NAMES list");
}

int	Irc::parse_file()
{
	std::string					content;
	std::string					line;
	std::vector<std::string>	splitted_line;

	std::ifstream file("IRCd", std::ios::in);
	if (file.is_open())
	{
		while(std::getline(file,line))
		{
			if (!line.empty())
			{
				if (split(line)[0] == "//")
					continue ;
				splitted_line = split(line, "=");
				if (splitted_line.size() < 2)
				{
					std::cerr << "Parsing: IRCd: line error" << std::endl;
					return(2);
				}
				if (splitted_line[0] == "hostname")
					_oper_login.push_back(splitted_line[1]);
				else if (splitted_line[0] == "name")
					_oper_login.push_back(splitted_line[1]);
				else if (splitted_line[0] == "password")
					_oper_login.push_back(splitted_line[1]);
				else
				{
					std::cerr << "Parsing: IRCd: line error--" << std::endl;
					return(3);
				}
				if (_oper_login.size() == 3)
					return (0);
			}
		}
		file.close();
	}
	else
	{
		std::cerr << "can't open file" << std::endl;
		return (1);
	}
	return (-1);
}

void	Irc::empty_operLogin(void)
{
	_oper_login.resize(0);
}

void	Irc::set_operLoginByVec(std::vector<std::string> vector)
{
	_oper_login = vector;
}

void	Irc::set_serverStat(int stat)
{
	_serverStat = stat;
}

void	Irc::removeChannel( std::string name ) {
	if (isChannel(name)) {
		_channels.erase(findContainer(_channels.begin(), _channels.end(), getChannel(name)));
	}
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/

std::string Irc::get_password() const {
	return (_password);
}

std::vector<Client> &Irc::get_clients() {
	return (_clients);
}

const time_t	&Irc::get_start_time(void) const
{
	return (_start_time);
}

const std::string	&Irc::get_hostname(void) const
{
	return (_hostname);
}

const std::string	Irc::initialize_hostname(void){
	char buff[1024];

	buff[1023] = '\0';
	gethostname(buff, 1022);
	return (buff);
}

std::vector<Channel> *	Irc::getChannels()
{
	return ( &_channels );
}

Client	*Irc::getClient(Client &client) {
    if (findContainer(_clients.begin(), _clients.end(), client) != _clients.end())
        return &*findContainer(_clients.begin(), _clients.end() , client);
    return NULL;
}

size_t	Irc::get_client_index(Client const & client) const
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (client == _clients[i])
			return (i);
	}
	return (_clients.max_size());
}

Client	*Irc::get_client_by_nickname(std::string const & nickname)
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i].get_nickname() == nickname)
			return (&_clients[i]);
	}
	return (NULL);
}

std::vector<std::string>	Irc::get_operLogin(void)
{
	return (_oper_login);
}

Client	*Irc::getClient(std::string nick) {
    if (std::find(_clients.begin(), _clients.end(), nick) != _clients.end())
        return &*std::find(_clients.begin(), _clients.end() , nick);
    return nullptr;
}

int	Irc::get_serverStat(void)
{
	return (_serverStat);
}

/* ************************************************************************** */


bool	operator==(Channel const &channel, std::string const &otherName) {
	return (channel.getName() == otherName);
}