#include "Channel.class.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Channel::Channel( std::string nameset ) : _name(nameset) , _Flags(0) {}

Channel::Channel( std::string nameset , std::string topicset ) : _name(nameset) , _topic( topicset ), _Flags(0) {};

Channel::Channel( std::string nameset , int flagsSet ) : _name(nameset) , _Flags( flagsSet ){};

Channel::Channel( std::string nameset , std::string topicset , int flagsSet ) : _name(nameset) , _topic( topicset ) , _Flags( flagsSet ){};

/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Channel::~Channel() {};

/*
** --------------------------------- ACCESSOR ---------------------------------
*/

// Retrieve client flags or returns -1 if client cannot be found
int Channel::getClientFlags( Client &client ) const {
    return _clients.find( &client )  != _clients.end() ? _clients.find( &client )->second : -1;
};

bool Channel::flagged( int flag ) const {
    return ( _Flags & flag );
};

int Channel::getMaxSize() const {
    return ( _maxSize );
};

int Channel::getClientCount() const {
    return ( _clients.size() );
};

bool Channel::isInviteOnly() const {
    return ( flagged(INVITE_ONLY_F) );
};

bool Channel::isSecret() const {
    return ( flagged( SECRET_F ) );
};

bool Channel::isChannelFull() const
{
    if ( !flagged(CLIENT_LIMIT_F) )
        return (false);
    if ( (int)_clients.size() >= (int)_maxSize )
        return ( true );
    return ( false );
}

bool Channel::isBanned( Client &client) const {
    return ( findIn(client, _bannedClients) != NULL );
};

std::string const &Channel::getName() const {
    return _name;
}

bool Channel::isOperator(Client &client) const {
    return _clients.find( &client )->second & CLIENT_OPERATOR_F;
}

bool Channel::isInChannel(Client &client) const {
    return _clients.find( &client ) != _clients.end();
}

std::string const &Channel::getTopic() const {
    return _topic;
}

std::string const Channel::chanModeString( bool is )
{
	std::string res;

	if ( ((_Flags & BAN_F) ? true : false) == is )
	{
		res += "b";
	}
	if ( ((_Flags & CLIENT_LIMIT_F) ? true : false) == is )
	{
		res += "l";
	}
	if ( ((_Flags & INVITE_ONLY_F) ? true : false) == is )
	{
		res += "i";
	}
	if ( ((_Flags & SECRET_F) ? true : false) == is )
	{
		res += "s";
	}
	if ( ((_Flags & PROTECTED_F) ? true : false) == is )
	{
		res += "t";
	}
	if ( ((_Flags & NO_EXTERNAL_MESSAGES_F) ? true : false) == is )
	{
		res += "n";
	}

	if (!res.empty())
	{
		if (is)
			res = "+" + res;
		else
			res = "-" + res;
	}
	return (res);
}

/*
** --------------------------------- METHODS ----------------------------------
*/

void Channel::bannedClientsList( Client & client )
{
	for (size_t i = 0; i < _bannedClients.size(); i++)
	{
		client.sendMessageToClient( "367 " + client.get_nickname() + " " + getName() + " " + _bannedClients[i]->get_nickname());
	}
	client.sendMessageToClient( "368 " + client.get_nickname() + " " + getName() + " :End of channel ban list");
}

bool isNumber(const std::string& s)
{
    for (size_t i = 0 ; i < s.size() ; i++) {
        if (std::isdigit(s[i]) == 0)
            return false;
    }
    return true;
}

void Channel::setModeFromString( Irc & irc , std::string modes , std::vector<std::string> & args , int & argStatus)
{
	if (modes.empty())
		return ;
	if ( modes.find("t") != std::string::npos )
		_Flags = _Flags | PROTECTED_TOPIC_F;
	if ( modes.find("n")  != std::string::npos )
		_Flags = _Flags | NO_EXTERNAL_MESSAGES_F;
	if ( modes.find("s")  != std::string::npos )
		_Flags = _Flags | SECRET_F;
	if ( modes.find("i")  != std::string::npos )
		_Flags = _Flags | INVITE_ONLY_F;
	if ( modes.find("l")  != std::string::npos )
	{
		if ((int)args.size() >= argStatus + 1 && isNumber(args[argStatus]))
		{
			_Flags = _Flags | CLIENT_LIMIT_F;
			setMaxSize(stoi(args[argStatus]));
			argStatus++;
		}
	}
	if ( modes.find("o")  != std::string::npos )
	{
		if ((int)args.size() >= argStatus + 1 && irc.getClient(args[argStatus]) && isInChannel(*irc.getClient(args[argStatus])))
		{
			_clients.find( irc.getClient(args[argStatus]) )->second = CLIENT_OPERATOR_F;
			argStatus++;
		}
	}
	if ( modes.find("b")  != std::string::npos )
	{
		if ((int)args.size() >= argStatus + 1 && irc.getClient(args[argStatus]))
		{
			_Flags = _Flags | BAN_F;
			banClient(*(irc.getClient(args[argStatus])));
			argStatus++;
		}
	}
}

void Channel::unModeFromString( Irc & irc , std::string modes , std::vector<std::string> & args , int & argStatus)
{
	if (modes.empty())
		return ;
	if ( modes.find("t")  != std::string::npos )
		_Flags = _Flags ^ PROTECTED_TOPIC_F;
	if ( modes.find("n")  != std::string::npos )
		_Flags = _Flags ^ NO_EXTERNAL_MESSAGES_F;
	if ( modes.find("s")  != std::string::npos )
		_Flags = _Flags ^ SECRET_F;
	if ( modes.find("i")  != std::string::npos )
		_Flags = _Flags ^ INVITE_ONLY_F;
	if ( modes.find("l")  != std::string::npos )
		_Flags = _Flags ^ CLIENT_LIMIT_F;
	if ( modes.find("o")  != std::string::npos )
	{
		if ((int)args.size() >= argStatus + 1 && irc.getClient(args[argStatus]) && isOperator(*irc.getClient(args[argStatus])))
			_clients.find(irc.getClient(args[argStatus]))->second = 0;
		argStatus++;
	}
	if ( modes.find("b")  != std::string::npos )
	{
		if ((int)args.size() >= argStatus + 1 && irc.getClient(args[argStatus]) && isBanned(*irc.getClient(args[argStatus])))
			unBanClient(*irc.getClient(args[argStatus]));
		argStatus++;
		if ( !_bannedClients.size() )
			_Flags = _Flags ^ BAN_F;
	}
}

// Finds client in a vector
Client *Channel::findIn(Client &client, std::vector<Client *> vec) const {
    if (findContainer(vec.begin(), vec.end() , &client) != vec.end())
        return *findContainer(vec.begin(), vec.end() , &client);
    return NULL;
}

int Channel::unBanClient( Client &client ) {
    if ( isBanned(client) )
    {
		for (size_t i = 0; i < _bannedClients.size(); i++)
		{
			if (*_bannedClients[i] == client)
			{
        		_bannedClients.erase(_bannedClients.begin() + i);
       			 return (1);
			}
		}
    }
    return (0);
};

int Channel::banClient( Client &client ) {
    if ( !isBanned(client) )
    {
		if (isInChannel(client))
        	_clients.erase(_clients.find( &client ));
        _bannedClients.push_back( &client );
        return (1);
    }
    return (0);
};

int Channel::removeClient( Client &client ) {
    if ( isInChannel(client) ) {
        _clients.erase( &client );
        return (1);
    }
    return (0);
};

int Channel::addClient( Client &client )
{
    if ( !isInChannel(client) )
    {
        _clients.insert( std::make_pair< Client *, int >( &client, 0 ) );
        return (1);
    }
    return (0);
};

int Channel::addClient( Client &client , int flags )
{
    if ( !isInChannel(client) )
    {
        _clients.insert( std::make_pair< Client *, int >( &client, flags ) );
        return (1);
    }
    return (0);
};

void Channel::setTopic( std::string const & newtopic )
{
    _topic = newtopic;
};

void Channel::setMaxSize( int newsize )
{
    _maxSize = newsize;
};

void Channel::sendClientsName( Client &client )
{
    std::string status(" ");
    // test status channel flags;
    for ( std::map< Client * , int >::iterator pair = _clients.begin() ; pair != _clients.end() ; pair++ )
    {
        std::string pref = "";
        if ( isOperator(*(pair->first)) )
            pref = "@";
        client.sendMessageToClient( "353 " + client.get_nickname() + " " + status  + getName() +  " :" + pref + pair->first->get_nickname() );
        //  "<client> <symbol> <channel> :[prefix]<nick>{ [prefix]<nick>}"
    }
    client.sendMessageToClient( "366 " + client.get_nickname() + " " + getName() +  " :End of /NAMES list");
}

void Channel::sendMessageToClients( std::string const & message )
{
    for ( std::map< Client * , int >::iterator pair = _clients.begin() ; pair != _clients.end() ; pair++ )
    {
        pair->first->sendMessageToClient(  message );
    }
}

void Channel::sendMessageToClientsByName( std::string const & message )
{
    for ( std::map< Client * , int >::iterator pair = _clients.begin() ; pair != _clients.end() ; pair++ )
    {
        pair->first->sendMessageToClient( ":" + pair->first->get_nickname() +" " + message );
    }
}
void Channel::sendMessageToClientsExcept( std::string const & message , Client & client)
{
    for ( std::map< Client * , int >::iterator pair = _clients.begin() ; pair != _clients.end() ; pair++ )
    {
		if (*pair->first != client)
        	pair->first->sendMessageToClient( message );
    }
}

void Channel::sendMessageToClientsOpExcept( std::string const & message , Client & client)
{
    for ( std::map< Client * , int >::iterator pair = _clients.begin() ; pair != _clients.end() ; pair++ )
    {
		if (*pair->first != client and isOperator(*pair->first))
        	pair->first->sendMessageToClient( message );
    }
}

bool    Channel::operator==(Channel const &other) const {
	return (_name == other.getName());
}

bool    Channel::operator!=(Channel const &other) const {
	return (_name != other.getName());
}

