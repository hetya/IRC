#ifndef __IRC_HPP__
# define __IRC_HPP__

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
#include <map>
#include <numeric>

#include <Enums.hpp>
#include "Client.class.hpp"
#include "Channel.class.hpp"

#define	NETWORKNAME "LesSingesSurLaBranche"
#define	SERVERNAME "LaBranche"
#define	SERVERVERSION "1"

#define LISTEN_BACKLOG 50

#define MAX_TARGETS_PRIVMSG 10
#define MAX_CLIENTS 100
#define BUFFER_SIZE 4097
#define MAX_CHANNEL 100

#define OK_SERV_STAT 0
#define QUIT_SERV_STAT 1

class Client;

typedef int (*CommandFunction)( Client & , std::vector<std::string> );

class Irc
{
private:
	int const					_port;
	std::string const			_password;
	std::string	const			_hostname;
	time_t	const				_start_time;
	std::vector<Client>			_clients;
	std::vector<Channel>		_channels;
	std::vector<std::string>	_oper_login;
	int							_serverStat;
public:
	std::map<std::string , CommandFunction> commands;
	std::map<std::string , CommandFunction> unregisteredCommands;
	Irc(char const *port, char const *password);
	~Irc();

	const std::string			initialize_hostname(void);
	Channel &					addChannel( std::string name );
	void						removeChannel( std::string name );
	int							parse_file();
	void						empty_operLogin(void);
	void						sendClientsName( Client &client );
	void						sendFatalError(std::string reason );
	void						sendMessageToClients(std::string message );

	void						set_operLoginByVec(std::vector<std::string>);
	void						set_serverStat(int stat);

	std::string					get_password() const;
	std::vector<Client>			&get_clients() ;
	const time_t				&get_start_time(void) const;
	const std::string			&get_hostname(void) const;
	Channel &					getChannel( std::string name );
	std::vector<Channel> *		getChannels();
	Client 						*getClient(Client &client);
	Client						*get_client_by_nickname(std::string const & nickname);
	size_t						get_client_index(Client const & client) const;
	std::vector<std::string>	get_operLogin(void);
	Client 						*getClient( std::string nick ); 
	int							get_serverStat(void);

	bool						isChannel( std::string name );
};

void							process_line(Irc &irc, Client &client);
void							ircServerErrorsHandling(IrcErrors err, struct pollfd *fds);
int								count_digit(int nb);
void							setCommands( std::map<std::string , CommandFunction> & commands);
void							setUnregisteredCommands( std::map<std::string , CommandFunction> & commands);
std::vector<std::string>		split(std::string const &s, std::string sep = " ");
std::string						vecStringJoin(std::vector<std::string>::const_iterator start, std::vector<std::string>::const_iterator end);
int 							oper(Client & client, std::vector<std::string> args);
int 							op_kill(Client & client, std::vector<std::string> args);
int 							op_rehash(Client & client, std::vector<std::string> args);
int 							op_squit(Client & client, std::vector<std::string> args);
bool							check_args(std::string port, std::string password);

bool							operator==(Channel const &, std::string const &);

int	receiveFromClient(Irc &irc, struct pollfd fd, Client &client);
int	sendToClient(Irc &irc, struct pollfd fd, Client &client);

std::string to_string(int val);

template<class InputIt, class T>
InputIt findContainer(InputIt first, InputIt last, const T& value)
{
    for (; first != last; ++first) {
        if (*first == value) {
            return first;
        }
    }
    return last;
}

#endif /* **************************************************** __IRC_HPP__ */
