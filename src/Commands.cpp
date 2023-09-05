#include "Irc.class.hpp"
#include <stdlib.h>

int pass(Client &  client, std::vector<std::string> args)
{
    if ( client.is_register() )
        return (client.send_code(ERR_ALREADYREGISTERED, args[0]));
    if (args.size() < 2 || args[1].empty())
    {
        client.set_kill();
		return (client.send_code(ERR_NEEDMOREPARAMS, args[0]));
	}
    if (args[1] != client.irc->get_password())
    {
        client.set_kill();
		return (client.send_code(ERR_NEEDMOREPARAMS, args[0]));
	}
    else
        client.registerFlag(REG_PASS_F);
    return (0);
}

int nick(Client &  client, std::vector<std::string> args)
{
    if (!client.is_pass()) {
        return (0);
    }
	if (client.set_nickname(*(client.irc), args[1]) == 0)
    {
        if (client._register == (REG_PASS_F | REG_USER_F))
            client.registration();
        client.registerFlag(REG_NICK_F);
    }
    return (0);
}

int user(Client &  client, std::vector<std::string> args)
{
    if (!client.is_pass())
        return (0);
    if ( client.is_register()) {
        return (client.send_code(ERR_ALREADYREGISTERED));
    }
    if (args.size() < 2)
        return (client.send_code(ERR_NEEDMOREPARAMS , args[0]));
	if (client.set_username(args) == 0)
		client.registerFlag(REG_USER_F);
    if (client.is_register())
        client.registration();
    return (0);
}

int join(Client &  client, std::vector<std::string> args)
{
    if (args.size() <= 1)
        return (client.send_code(ERR_NEEDMOREPARAMS, args[0]));
    if (args[1].empty()
        || (*(args[1].begin() ) != '&'
        and *(args[1].begin() ) != '#')
        || args[1].find(" ") != std::string::npos
        || args[1].find("^") != std::string::npos
        || args[1].find(",") != std::string::npos )
        return (client.send_code(ERR_BADCHANMASK, args[1]));
    if (args.size() > 2)
    {
        if ( args.size() > 3 || (args[1][0] != '&' and args[1][0] != '#') || client.irc->getChannels()->size() >= MAX_CHANNEL )
            return (client.send_code(ERR_TOOMANYCHANNELS, args.back()));
    }
    Channel * channel;
    if (client.irc->isChannel(args[1]))
    {
        channel = &client.irc->getChannel(args[1]);
        if ( channel->isBanned( client ) )
            return (client.send_code(ERR_BANNEDFROMCHAN , args[1]));
        if ( channel->isChannelFull() )
            return (client.send_code(ERR_CHANNELISFULL , args[1]));
        if ( channel->isInviteOnly() && !client.isInviteOn( channel ) )
            return (client.send_code(ERR_INVITEONLYCHAN , args[1]));
        if ( channel->isInChannel( client ) )
            return (0);
        channel->addClient( client );
    }
    else
    {
        channel = &client.irc->addChannel(args[1]);
        channel->addClient( client , CLIENT_OPERATOR_F );
    }
    
    if (!channel->getTopic().empty())
        client.sendMessageToClient( "332 " + client.get_nickname() + " " + channel->getName() + " :" + channel->getTopic() );
    channel->sendMessageToClients( ":" + client.get_nickname() + " JOIN " + channel->getName()  );
    channel->sendClientsName( client );

    return (0);
}

int topic(Client &  client, std::vector<std::string> args)
{
    if (args.size() <= 1)
        return (client.send_code(ERR_NEEDMOREPARAMS , args[0]));
    if (args[1].empty() 
        || (args[1][0] != '&'
        and args[1][0] != '#')
        || args[1].find(" ") != std::string::npos
        || args[1].find("^") != std::string::npos
        || args[1].find(",") != std::string::npos )
        return (client.send_code(ERR_NOSUCHCHANNEL, args[1]));
    Channel * channel;
    if (client.irc->isChannel(args[1]))
    {
        channel = &client.irc->getChannel(args[1]);
        if ( !channel->isInChannel( client ) )
            return (client.send_code(ERR_NOTONCHANNEL, args[1]));
        if (args.size() == 2)
        {
            if (!channel->getTopic().empty())
                client.sendMessageToClient("332 " + client.get_nickname() + " " + channel->getName() + " :" + channel->getTopic() );
            else
                client.sendMessageToClient( "331 " + client.get_nickname() + " " + channel->getName() + " :No topic is set" );
            return (0);
        }
        if ( channel->flagged(PROTECTED_TOPIC_F) && !channel->isOperator( client ) )
            return (client.send_code(ERR_CHANOPRIVSNEEDED, channel->getName()));
        channel->setTopic(args[2]);
        channel->sendMessageToClientsByName( args[0] + " " + args[1] + " " + args[2] );
        return (0);
    }
    else
        return (client.send_code(ERR_NOSUCHCHANNEL, args[1]));
}

int ping(Client & client, std::vector<std::string> args)
{
    if (args.size() < 2)
        return (client.send_code(ERR_NEEDMOREPARAMS , args[0]));
    std::string const   response = "PONG " + args[1];
    client.sendMessageToClient(response);
	std::cout << response << std::endl;
	return (0);
}

int part(Client & client, std::vector<std::string> args)
{
    Irc &irc = *client.irc;

    if (args.size() < 2)
        return (client.send_code(ERR_NEEDMOREPARAMS, args[0]));
    std::vector<std::string> channels = split(args[1], ",");

    for (size_t i = 0; i < channels.size(); i++) {
        // Channel exists but client is not on it
        if (irc.isChannel(channels[i]) and !irc.getChannel(channels[i]).isInChannel(client)) {
            client.sendMessageToClient("442 " + client.get_nickname() + " " + channels[i] + " :You're not on that channel");
        }
        // Channel does not exist
        else if (!irc.isChannel(channels[i])) {
            client.sendMessageToClient("403 " + client.get_nickname() + " " + channels[i] + " :No such channel");
        }
        // Client leaves
        else {
            Channel &channel = irc.getChannel(channels[i]);
            channel.sendMessageToClients(":" + client.get_nickname() + " PART " + channels[i] + (args.size() > 2 ? " because " + args[2] : ""));
            channel.removeClient(client);

            // If user was last on the channel, destroy it
            if (channel.getClientCount() == 0) {
                irc.removeChannel(channel.getName());
            }
        }
    }

	return (0);
}

int privmsg(Client & client, std::vector<std::string> args)
{
    Irc &irc = *client.irc;
    bool        toOp = false;

    // Missing target
    if (args.size() < 2)
        return (client.send_code(ERR_NEEDMOREPARAMS , args[0]));
    // Missing text to send
    if (args.size() < 3)
        return (client.send_code(ERR_NOTEXTTOSEND));

    std::vector<std::string> targets = split(args[1], ",");

    // Too many targets to send message
    if (targets.size() > MAX_TARGETS_PRIVMSG)
        return client.send_code(ERR_TOOMANYTARGETS);

    for (size_t i = 0; i < targets.size(); i++) {
        if (targets[i][0] == '@') {
            targets[i] = targets[i].substr(1);
            toOp = true;
            if (targets[i][0] != '#' and targets[i][0] != '&') {
                continue;
            }
        }
        // Channel exists in channels list
        if (irc.isChannel(targets[i])) {
            Channel &channel = irc.getChannel(targets[i]);
            // Client is on channel, message can be send
            if (irc.getChannel(targets[i]).isInChannel(client) || !channel.flagged(NO_EXTERNAL_MESSAGES_F) ) {
                if (toOp) {
                    channel.sendMessageToClientsOpExcept(":" + client.getName() + " " + args[0] + " " + targets[i] + " :" + vecStringJoin(args.begin() + 2, args.end()) , client);
                } else {
                    channel.sendMessageToClientsExcept(":" + client.getName() + " " + args[0] + " " + targets[i] + " :" + vecStringJoin(args.begin() + 2, args.end()) , client);
                }
            }
            // Client is not on channel and not banned from channel
            else if (channel.isBanned(client)) {
                client.send_code(ERR_CANNOTSENDTOCHAN, targets[i]);
            }
            // If client is banned, do nothing
        }
        // Channel name is another user
        else if (irc.get_client_by_nickname(targets[i]) != NULL and (*irc.get_client_by_nickname(targets[i]) != client)) {
            Client  *other = irc.get_client_by_nickname(targets[i]);

            // Sends message to the other user
            other->sendMessageToClient(":" + client.getName() + " " + args[0] + " " + other->get_nickname() + " :" + vecStringJoin(args.begin() + 2, args.end()));
        }
        // Channel or other user not recognized
        else {
            // Channel not recognized
            if (targets[i][0] == '&' or targets[i][0] == '#') {
                client.sendMessageToClient("403 " + client.getName() + " " + targets[i] + " :No such channel");
            }
            // User not recognized
            else {
                client.sendMessageToClient("401 " + client.get_nickname() + " " + targets[i] + " :No such nick");
            }
        }
    }
    return 0;
}

int notice(Client & client, std::vector<std::string> args)
{
    Irc &irc = *client.irc;
    bool        toOp = false;

    if (args.size() < 3)
        return 0;

    std::vector<std::string> targets = split(args[1], ",");

    // Too many targets to send message
    if (targets.size() > MAX_TARGETS_PRIVMSG)
        return 0;

    for (size_t i = 0; i < targets.size(); i++) {
        if (targets[i][0] == '@') {
            targets[i] = targets[i].substr(1);
            toOp = true;
            if (targets[i][0] != '#' and targets[i][0] != '&') {
                continue;
            }
        }
        // Channel exists in channels list
        if (irc.isChannel(targets[i])) {
            Channel &channel = irc.getChannel(targets[i]);
            // Client is on channel, message can be send
            if (irc.getChannel(targets[i]).isInChannel(client) || !channel.flagged(NO_EXTERNAL_MESSAGES_F) ) {
                if (toOp) {
                    channel.sendMessageToClientsOpExcept(":" + client.getName() + " " + args[0] + " " + targets[i] + " :" + vecStringJoin(args.begin() + 2, args.end()) , client);
                } else {
                    channel.sendMessageToClientsExcept(":" + client.getName() + " " + args[0] + " " + targets[i] + " :" + vecStringJoin(args.begin() + 2, args.end()) , client);
                }
            }
            // If client is banned, do nothing
        }
        // Channel name is another user
        else if (irc.get_client_by_nickname(targets[i]) != NULL and (*irc.get_client_by_nickname(targets[i]) != client)) {
            Client  *other = irc.get_client_by_nickname(targets[i]);

            // Sends message to the other user
            other->sendMessageToClient(":" + client.getName() + " " + args[0] + " " + other->get_nickname() + " :" + vecStringJoin(args.begin() + 2, args.end()));
        }
    }
    return 0;
}

int names(Client & client, std::vector<std::string> args)
{
    Irc &irc = *client.irc;

    if (args.size() < 1)
        return (client.send_code(ERR_NEEDMOREPARAMS , args[0]));
    if (args.size() == 1)
    {
        irc.sendClientsName( client );
        return (0);
    }
    std::vector<std::string> channels = split(args[1], ",");

    for (size_t i = 0; i < channels.size(); i++) {
        // Channel does not exist
        if (!irc.isChannel(channels[i]))
            client.sendMessageToClient( "366 " + client.get_nickname() + " " + channels[i] +  " :End of /NAMES list");
        // Channel is set to secret mode
        else if (irc.getChannel(channels[i]).isSecret() && !irc.getChannel(channels[i]).isInChannel(client))
            client.sendMessageToClient( "366 " + client.get_nickname() + " " + channels[i] +  " :End of /NAMES list");
        // Channel exists but client is not on it
        else
            irc.getChannel(channels[i]).sendClientsName( client );
        
    }

	return (0);
}

int list(Client & client, std::vector<std::string> args)
{
    Irc &irc = *client.irc;

    if (args.size() < 1)
        return (client.send_code(ERR_NEEDMOREPARAMS , args[0]));

    std::vector<std::string> channels;

    if (args.size() == 1)
    {
        for (int j = 0 ; j < (int)irc.getChannels()->size() ;j++ )
            channels.push_back( (*irc.getChannels())[j].getName() );
    }
    else
        channels = split(args[1], ",");

    client.sendMessageToClient( "321 " + client.get_nickname() + " Channel :Users  Name");
    for (size_t i = 0; i < channels.size(); i++) {

        if (irc.isChannel(channels[i]) && !irc.getChannel(channels[i]).isSecret())
            client.sendMessageToClient(  "322 " + client.get_nickname() + " " + irc.getChannel(channels[i]).getName() + " " + to_string( irc.getChannel(channels[i]).getClientCount() ) + " :" + irc.getChannel(channels[i]).getTopic() );
        
    }
    client.sendMessageToClient( "323 " + client.get_nickname() + " :End of /LIST");

	return (0);
}

int invite(Client & client, std::vector<std::string> args)
{
    Irc &irc = *client.irc;

    if ( args.size() == 1 )
    {
        client.showChannelsInvites();
        return (0);
    }

    if (args.size() < 3)
        return (client.send_code(ERR_NEEDMOREPARAMS , args[0]));

    if ( !irc.isChannel( args[2] ) )
        return (client.send_code(ERR_NOSUCHCHANNEL, args[2]));

    Channel & channel = irc.getChannel( args[2] );

    if (!channel.isInChannel( client ))
        return (client.send_code(ERR_NOTONCHANNEL, args[2]));

    if (!channel.isOperator( client ))
        return (client.send_code(ERR_CHANOPRIVSNEEDED, args[2]));

    Client * invitedClient = irc.getClient(args[1]);

    if (!invitedClient)
        return (0);
    if (channel.isInChannel( *invitedClient ))
        return (client.send_code(ERR_USERONCHANNEL, invitedClient->get_nickname() + "," + channel.getName()));

    invitedClient->addInvite( &channel );
    client.sendMessageToClient( "341 " + client.get_nickname() + " " + args[1] + " " + args[2]);
    invitedClient->sendMessageToClient( ":" + client.get_nickname() + " INVITE " + invitedClient->get_nickname() + " " + args[2]);

	return (0);
}

int kick(Client & client, std::vector<std::string> args)
{
    Irc &irc = *client.irc;

    if (args.size() < 4)
        return (client.send_code(ERR_NEEDMOREPARAMS , args[0]));

    if ( !irc.isChannel( args[1] ) )
        return (client.send_code(ERR_NOSUCHCHANNEL, args[1]));
    Channel & channel = irc.getChannel( args[1] );

    if (!channel.isInChannel( client ))
        return (client.send_code(ERR_NOTONCHANNEL, args[1]));

    if (!channel.isOperator( client ))
        return (client.send_code(ERR_CHANOPRIVSNEEDED, args[1]));

    std::vector<std::string> clients = split(args[2], ",");
    std::string comment = "";
    if (args.size() >= 5)
    {
        comment += " ";
        comment += args[4];
    }
    for (size_t i = 0; i < clients.size(); i++) {

        if (irc.getClient( clients[i] ) && channel.isInChannel( *irc.getClient( clients[i] )))
        {
            channel.sendMessageToClients(  ":" + client.get_nickname() + " KICK " + channel.getName() + " " + clients[i] + comment );
            channel.removeClient( *irc.getClient( clients[i] ) );
        }
        
    }
	return (0);
}

int quit(Client & client, std::vector<std::string> args)
{
	client.sendMessageToClient( "ERROR :Quit" );
	client.set_kill();

	 if (args.size() != 1)
		client.exitraison = vecStringJoin(args.begin() + 1, args.end());

	return (0);
}

int mode(Client & client, std::vector<std::string> args)
 {
    Irc &irc = *client.irc;

    if (args.size() < 2)
        return (client.send_code(ERR_NEEDMOREPARAMS , args[0]));

    if ( *args[1].begin() == '#' || *args[1].begin() == '&')
	{
		if (!irc.isChannel(args[1]))
			return (client.send_code(ERR_NOSUCHCHANNEL, args[1]));

    	Channel & channel = irc.getChannel( args[1] );

		if (args.size() == 2)
		{
			std::string chanModeArg;
			if (channel.flagged(CLIENT_LIMIT_F))
				chanModeArg = " " + to_string(channel.getMaxSize());
			client.sendMessageToClient( "324 " + client.get_nickname() + " " + args[1] + " " + channel.chanModeString(true) + channel.chanModeString(false) + chanModeArg);
			if (channel.flagged(BAN_F))
			{
				channel.bannedClientsList( client );
			}
		}
		else
        {
			int argStatus = 3;
            if (!channel.isOperator( client ))
                 return (client.send_code(ERR_CHANOPRIVSNEEDED, channel.getName()));
				 try{
					channel.setModeFromString(irc, args[2].substr(args[2].find("+") , (args[2].find("-") > args[2].find("+") ? args[2].find("-") : std::string::npos) )  , args , argStatus );
				 }
				 catch(const std::exception& e)
				 {
				 }
				 try{
					channel.unModeFromString(irc,args[2].substr(args[2].find("-") , (args[2].find("+") > args[2].find("-") ? args[2].find("+") : std::string::npos) ) , args , argStatus );
				 }
				 catch(const std::exception& e)
				 {
				 }
        }
		return (0);
	}

    if ( !irc.getClient( args[1] ) )
        return (client.send_code(ERR_NOSUCHNICK, args[1]));

    if ( client.get_nickname() != args[1] )
        return (client.send_code(ERR_USERSDONTMATCH));

    if (args.size() == 2)
	{
		if (client.is_op())
			client.sendMessageToClient( "221 " + client.get_nickname() + " " + "+o");
		else
			client.sendMessageToClient( "221 " + client.get_nickname());
	}
    else if (args.size() == 4)
	{
		if (args[2] != "+o")
			return (client.send_code(ERR_UMODEUNKNOWNFLAG));
		if (!irc.getClient( args[3] ) )
			return (client.send_code(ERR_NOSUCHNICK, args[3]));
		if (!client.is_op())
			return 0;
		(irc.getClient( args[3] ))->setop();
	}
	else
		return (client.send_code(ERR_UMODEUNKNOWNFLAG));

	return (0);
}

int cap(Client & client, std::vector<std::string> args) {
    (void)client;
    (void)args;
    
    return 0;
}

void setCommands( std::map<std::string , CommandFunction> & commands)
{
    commands.insert(std::make_pair<std::string , CommandFunction>("PING" , ping));
    commands.insert(std::make_pair<std::string , CommandFunction>("PASS" , pass));
    commands.insert(std::make_pair<std::string , CommandFunction>("NICK" , nick));
    commands.insert(std::make_pair<std::string , CommandFunction>("USER" , user));
    commands.insert(std::make_pair<std::string , CommandFunction>("JOIN" , join));
    commands.insert(std::make_pair<std::string , CommandFunction>("PART" , part));
    commands.insert(std::make_pair<std::string , CommandFunction>("PRIVMSG" , privmsg));
    commands.insert(std::make_pair<std::string , CommandFunction>("TOPIC" , topic));
    commands.insert(std::make_pair<std::string , CommandFunction>("NAMES" , names));
    commands.insert(std::make_pair<std::string , CommandFunction>("LIST" , list));
    commands.insert(std::make_pair<std::string , CommandFunction>("NOTICE" , notice));
    commands.insert(std::make_pair<std::string , CommandFunction>("OPER" , oper));
    commands.insert(std::make_pair<std::string , CommandFunction>("KILL" , op_kill));
    commands.insert(std::make_pair<std::string , CommandFunction>("REHASH" , op_rehash));
    commands.insert(std::make_pair<std::string , CommandFunction>("SQUIT" , op_squit));
    commands.insert(std::make_pair<std::string , CommandFunction>("INVITE" , invite));
    commands.insert(std::make_pair<std::string , CommandFunction>("KICK" , kick));
    commands.insert(std::make_pair<std::string , CommandFunction>("MODE" , mode));
    commands.insert(std::make_pair<std::string , CommandFunction>("QUIT" , quit));
    commands.insert(std::make_pair<std::string , CommandFunction>("CAP" , cap));
}

// Commands that can be executed without being registered
void setUnregisteredCommands( std::map<std::string , CommandFunction> & commands) {
    commands.insert(std::make_pair<std::string , CommandFunction>("PASS" , pass));
    commands.insert(std::make_pair<std::string , CommandFunction>("NICK" , nick));
    commands.insert(std::make_pair<std::string , CommandFunction>("USER" , user));
    commands.insert(std::make_pair<std::string , CommandFunction>("CAP" , cap));
}
