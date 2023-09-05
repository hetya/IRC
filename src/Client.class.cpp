
#include "Client.class.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Client::Client(Irc * ircset, std::string ip ) : _nickname(""), _username(""), _pass(""),  _ip(ip), _op(0), _willBeKill(0), _register(0), irc(ircset)
{
}


/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Client::~Client(void)
{
}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/

// Will be used by map.find in Channel class for example
bool		Client::operator<(Client const &other) const {
	return (this->_nickname == other.get_nickname());
}

bool		Client::operator==(Client const &other) const {
	return (this->_nickname == other.get_nickname());
}

bool		Client::operator!=(Client const &other) const {
	return !(this->_nickname == other.get_nickname());
}

/*
** --------------------------------- METHODS ----------------------------------
*/

void Client::addInvite( Channel * channel)
{
	if (!isInviteOn(channel))
		_invites.push_back(channel);
}

void	Client::registration()
{
	send_code(RPL_WELCOME);
	send_code(RPL_YOURHOST);
	send_code(RPL_CREATED);
	send_code(RPL_MYINFO);
	send_code(RPL_ISUPPORT);
}

void	Client::add_to_buffRead(const std::string &buff)
{
	_buffRead += buff;
}

bool	Client::line_in_buffRead(void) {
	return (_buffRead.find('\n') != std::string::npos);
}

std::string	Client::get_line(void)
{
	std::string	line;
	size_t rn;
	size_t n;

	rn = _buffRead.find("\r\n");
	n = _buffRead.find('\n');

	// No end of line found
	if (n == std::string::npos && rn == std::string::npos)
		return ("");
	// Found \r\n
	else if (rn < n) {
		line = _buffRead.substr(0, rn);
		_buffRead.erase(0, rn + 2);
	}
	// Found only \n
	else {
		line = _buffRead.substr(0, n);
		_buffRead.erase(0, n + 1);
	}
	return (line);
}

void		Client::showChannelsInvites()
{
    for ( std::vector< Channel * >::iterator pair = _invites.begin() ; pair != _invites.end() ; pair++ )
    {
        sendMessageToClient( ":" + get_nickname() + " " + (*pair)->getName());
    }
    sendMessageToClient( ":" + get_nickname() +  " :End of /INVITE list");
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/

void							Client::setop()
{
	_op = 1;
}

bool		Client::isThereSomethingToSend() const {
	return !_buffWrite.empty();
}

// They MUST NOT contain any of the following characters: space (' '), comma (','),
// asterisk ('*'), question mark ('?'), exclamation mark ('!'), at sign ('@'), any dot ('.').
// They MUST NOT start with any of the following characters: dollar ('$'), colon (':').
// They MUST NOT start with a character listed as a channel type prefix('#', '&').
int	Client::set_nickname(Irc &irc, std::string nickname)
{
	std::vector<Client> other_clients;
	std::string not_allowed = " ,*?!@.";
    std::string bad_prefix = "$:#&";
	size_t		i;
	int			char_not_allowed;

	if(nickname.empty())
		return (send_code(ERR_NONICKNAMEGIVEN));
	if (nickname.length() > NICKLEN)
		return (send_code(ERR_ERRONEUSNICKNAME, nickname));
	i = 0;
	for(; nickname[i]; i++)
	{
		char_not_allowed = 0;
		for(int j = 0; not_allowed[j]; j++)
		{
			if (nickname[i] == not_allowed[j])
			char_not_allowed = 1;
		}
		if (char_not_allowed != 0)
			return (send_code(ERR_ERRONEUSNICKNAME, nickname));
	}
	if (bad_prefix.find(nickname[0]) != std::string::npos)
        return (send_code(ERR_ERRONEUSNICKNAME, nickname));

	other_clients = irc.get_clients();
	i = 0;
	for(;i < other_clients.size(); i++)
		if (nickname == other_clients[i].get_nickname())
			return (send_code(ERR_NICKNAMEINUSE, nickname));

	std::string last  = _nickname;
	_nickname = nickname;

	// if (last.empty() && (_register | REG_NICK_F))
	// 	irc.sendMessageToClients( ":" + _nickname );
	if (!last.empty())
		irc.sendMessageToClients( ":" + last + " NICK " + _nickname ); 
	return (0);
}

int	Client::set_username(std::vector<std::string> args) {
	// Get username
	_username = args[1];

	// If using syntax `<username> 0 * <realname>`, search for realname and strip front `:`
	if (args.size() == 5 and args[2] == "0" and args[3] == "*")
		_realname = *(args[4].begin() + 1) == ':' ? args[4].substr(1) : args[4];

	// Truncate username in case it is too long
	if (_username.length() > USERLEN)
		_username = _username.substr(0, USERLEN - 1);
	return (0);
}

void	Client::set_kill(void)
{
	_willBeKill = 1;
}

void Client::registerFlag( long int f )
{
	_register = _register | f;
}

std::string Client::getbuff(void) const
{
	return ( _buffRead );
}

bool Client::is_register(void)
{
    return (_register == REG_G_F);
}

bool Client::is_pass(void) const
{
    return (_register & REG_PASS_F);
}

bool Client::is_nick(void) const
{
    return (_register & REG_PASS_F);
}

bool Client::is_user(void) const
{
    return (_register & REG_PASS_F);
}

std::string Client::get_nickname(void) const
{
	return (_nickname);
}

std::string Client::get_username(void) const
{
	return (_username);
}

std::string Client::get_realname(void) const
{
	return (_realname);
}

std::string	Client::getBuffRead(void) const
{
	return (_buffRead);
}

std::string	Client::getBuffWrite(void) const
{
	return (_buffWrite);
}

	bool							Client::isInviteOn( Channel * channel ) const
	{
		for (size_t i = 0; i < _invites.size(); i++)
		{
			if (_invites[i] == channel)
				return (true);
		}
		return (false);
	}

std::string	Client::getName(void) const
{
	return (_nickname + "!" + _username + "@" + _ip );
}

void	Client::sendMessageToClient(std::string const &str)  {
	_buffWrite += str + "\r\n";
}

void	Client::setBuffWrite(std::string const &str) {
	_buffWrite = str;
}

bool	Client::is_op(void) const
{
	return (_op);
}


int	Client::getChannelFlags(Channel const &channel) const {
	if (_channels.find(channel.getName()) != _channels.end())
		return _channels.find(channel.getName())->second;
	return -1;
}

std::vector<Channel *>	Client::get_client_channels(void)
{
	std::vector<Channel *> client_channels;

	for (size_t i = 0 ; i < irc->getChannels()->size(); i++)
	{
		if ((*irc->getChannels())[i].isInChannel(*this))
			client_channels.push_back(&(*irc->getChannels())[i]);
	}
	return (client_channels);
}

std::string const	&Client::get_ip(void) const
{
	return (_ip);
}

bool	Client::willBeKill(void) const
{
	return (_willBeKill);
}

/* ************************************************************************** */

bool	operator==(Client const &client, std::string const &nick) {
	return (client.get_nickname() == nick);
}
