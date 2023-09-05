#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__

#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <poll.h>
#include <Enums.hpp>
#include "Irc.class.hpp"

// Max len for username and nickname
#define USERLEN 15
#define NICKLEN 15

// Registration status
#define REG_PASS_F 0b0001
#define REG_NICK_F 0b0010
#define REG_USER_F 0b0100
#define REG_G_F 0b0111

class Irc;
class Channel;

class Client
{
private:
	std::string 				_nickname;
	std::string 				_username;
	std::string					_realname;
	std::string 				_pass;
	std::string					_ip;
	std::string					_buffRead;
	std::string					_buffWrite;
	// Map channel names to flags for each channel
	std::map< std::string, int>	_channels;
	time_t						_time_since_registration;
	bool						_op;
	std::vector< Channel * >	_invites;
	bool						_willBeKill;

public:
	int			_register;
	std::string exitraison;
	Irc * irc;
	pollfd fds[1];

	Client( Irc * ircset, std::string ip);
	~Client();

	void							registration();
	int 							send_code(IrcCodes code, std::string args = "");

	void							add_to_buffRead(const std::string &buff);
	bool							line_in_buffRead(void);
	std::string						get_line(void);
	void 							registerFlag( long int f );

	int								set_nickname(Irc &irc, std::string nickname);
	int								set_username(std::vector<std::string> args);
	void							set_time_since_registration(void);
	void							set_kill(void);
	void							addInvite( Channel * channel);

	std::string 					getbuff(void) const;
	int								getFd(void) const;
	pollfd							&getpollfd(void);
	std::string 					getName(void) const;
	std::string 					get_nickname(void) const;
	std::string 					get_username(void) const;
	std::string 					get_realname(void) const;
	std::string						getBuffRead(void) const;
	std::string						getBuffWrite(void) const;
	void							setBuffWrite(std::string const &str);
	void							setop();
	int								getChannelFlags(Channel const &channel) const;
	std::vector<Channel *>			get_client_channels(void);
	std::string const				&get_ip(void) const;

	bool							willBeKill(void) const;
	bool							is_register(void);
	bool							is_pass(void) const;
	bool							is_nick(void) const;
	bool							isInviteOn( Channel * channel ) const;
	bool							is_user(void) const;
	bool							isThereSomethingToSend() const;
	bool							is_op(void) const;
	

	bool							operator<(Client const &) const;
	bool							operator==(Client const &) const;
	bool							operator!=(Client const &) const;

	void							sendMessageToClient(std::string const &str);
	void							showChannelsInvites();
};

bool	operator==(Client const &client, std::string const &nick);

#endif /* **************************************************** __CLIENT_HPP__ */
