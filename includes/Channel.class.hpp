#ifndef __CHANNEL_HPP__
#define __CHANNEL_HPP__

#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include "Irc.class.hpp"
#include "Client.class.hpp"

class Irc;
class Client;

#define BAN_F ( 1 << 0 )
#define EXCEPTION_F ( 1 << 1 )
#define CLIENT_LIMIT_F ( 1 << 2 )
#define INVITE_ONLY_F ( 1 << 3 )
#define INVITE_EXCEPTION_F ( 1 << 4 )
#define KEY_F ( 1 << 5 )
#define MODERATED_F ( 1 << 6 )
#define SECRET_F ( 1 << 7 )
#define PROTECTED_F ( 1 << 8 )
#define NO_EXTERNAL_MESSAGES_F ( 1 << 9 )
#define PROTECTED_TOPIC_F ( 1 << 10 )

#define CLIENT_FOUNDER_F ( 1 << 0 )
#define CLIENT_PROTECTED_F ( 1 << 1 )
#define CLIENT_OPERATOR_F ( 1 << 2 )
#define CLIENT_HALFOP_F ( 1 << 3 )
#define CLIENT_VOICE_F ( 1 << 4 )

class Channel
{
    private:
        std::string _name;
        std::string _topic;
        std::map< Client *, int > _clients;
        std::vector< Client *> _bannedClients;
        int _Flags;
        int _maxSize;

        Client *findIn(Client &client, std::vector<Client *> vec) const;

    public:
        Channel( std::string nameset );
        Channel( std::string nameset , std::string topicset );
        Channel( std::string nameset , int flagsSet );
        Channel( std::string nameset , std::string topicset , int flagsSet );
        ~Channel();

        int addClient( Client & client );
        int addClient( Client & client , int flags );
        int banClient( Client & client );
        int unBanClient( Client & client );
        int removeClient( Client & client );

        void setTopic( std::string const & newtopic );
        void setMaxSize( int newsize );

        int getClientFlags( Client &client ) const;
        int getMaxSize() const;
        int getClientCount() const;
        std::string const &getTopic() const;
        std::string const &getName() const;
		void bannedClientsList( Client & client );

		bool flagged( int flag ) const;
        bool isBanned( Client &client) const;
        bool isInChannel(Client &client) const;
        bool isChannelFull() const;
        bool isInviteOnly() const;
        bool isSecret() const;
        bool isOperator(Client &client) const;
        
        void sendClientsName( Client &client );
        void sendMessageToClientsExcept( std::string const & message , Client & client);
        void sendMessageToClientsOpExcept( std::string const & message , Client & client);
        void sendMessageToClientsByName( std::string const & message );
        void sendMessageToClients( std::string const & message );
        std::string const chanModeString( bool is );
        void setModeFromString( Irc & irc , std::string modes , std::vector<std::string> & args , int & argStatus);
        void unModeFromString( Irc & irc , std::string modes , std::vector<std::string> & args , int & argStatus);

        bool    operator==(Channel const &other) const;
        bool    operator!=(Channel const &other) const;

};

#endif /* **************************************************** __CHANNEL_HPP__ */
