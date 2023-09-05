#include <sys/socket.h>
#include <iostream>
#include "Client.class.hpp"
#include <iomanip>
#include "Irc.class.hpp"

int Client::send_code(IrcCodes code, std::string args)
{
	std::vector<std::string>	arg;
	std::string str_send;

	if (!args.empty())
	{
		arg = split(args, ",");
	}
	str_send = to_string(code);
	if(count_digit(code) < 2)
		str_send.insert(0, "00");
	else if (count_digit(code) < 3)
		str_send.insert(0, "0");
	switch (code)
	{
	case 1:
	{
		str_send += " " + _nickname + " :Welcome to the " + NETWORKNAME + " Network, " + _nickname + "!" + _username + "@" + irc->get_hostname();
		break;
	}
	case 2:
	{
		str_send += " " + _nickname + " :Your host is " + SERVERNAME + ", running version " + SERVERVERSION;
		break;
	}
	case 3:
	{
		std::string time = asctime(localtime(&irc->get_start_time()));
		str_send += " " + _nickname + " :This server was created " + time.substr(0, time.length() - 1);
		break;
	}
	case 4:
	{
		str_send += " " + _nickname + " " + SERVERNAME + " " + SERVERVERSION + " " + "available user modes: operator; available channel modes: t, n, s, i; modes that takes parameter: l, b";
		break;
	}
	case 5:
	{
		str_send += " " + _nickname + " CHANLIMIT=" + to_string(MAX_CHANNEL) + " :are supported by this server";
		break;
	}
	case ERR_NEEDMOREPARAMS:
	{
		str_send += " " + _nickname + " " + arg[0] + " :Not enough parameters";
		break;
	}
	case ERR_PASSWDMISMATCH:
	{
		str_send += " " + _nickname + " :Password incorrect";
		break;
	}
	case ERR_NONICKNAMEGIVEN:
	{
		str_send += " " + _nickname + " :No nickname given";
		break;
	}
	case ERR_ERRONEUSNICKNAME:
	{
		str_send += " " + _nickname + " " + arg[0] + " :Erroneus nickname";
		break;
	}
	case ERR_NICKNAMEINUSE:
	{
		str_send += " " + _nickname + " " + arg[0] + " :Nickname is already in use";
		break;
	}
	case ERR_ALREADYREGISTERED:
	{
		str_send += " " + _nickname + " :You may not reregister";
		break;
	}
	case ERR_BADCHANMASK:
	{
		if (arg[0].empty()) //~~
			arg[0] = "";
		str_send += " " + arg[0] + " :Bad Channel Mask";
		break;
	}
	case ERR_TOOMANYCHANNELS:
	{
		str_send += " " + _nickname + " " + arg[0] + " :You have joined too many channels";
		break;
	}
	case ERR_BANNEDFROMCHAN:
	{
		str_send += " " + _nickname + " " + arg[0] + " :Cannot join channel (+b)";
		break;
	}
	case ERR_CHANNELISFULL:
	{
		str_send += " " + _nickname + " " + arg[0] + " :Cannot join channel (+l)";
		break;
	}
	case ERR_INVITEONLYCHAN:
	{
		str_send += " " + _nickname + " " + arg[0] + " :Cannot join channel (+i)";
		break;
	}
	case ERR_NOTREGISTERED:
	{
		str_send += " " + _nickname + " :You have not registered";
		break;
	}
	case ERR_NOTEXTTOSEND:
	{
		str_send += " " + _nickname + " :There is no text to send";
		break;
	}
	case ERR_TOOMANYTARGETS:
	{
		str_send += " " + _nickname + " :Too many target";
		break;
	}
	case ERR_CANNOTSENDTOCHAN:
	{
		str_send += " " + _nickname + " " + arg[0] + " :Cannot send to channel";
		break;
	}
	case ERR_NOSUCHCHANNEL:
	{
		str_send += " " + _nickname + " " + arg[0] + " :No such channel";
		break;
	}
	case ERR_USERONCHANNEL:
	{
		str_send += " " + _nickname + " " + arg[0] + " " + arg[1] + " :is already on channel";
		break;
	}
	case ERR_NOTONCHANNEL:
	{
		str_send += " " + _nickname + " " + arg[0] + " :You're not on that channel";
		break;
	}
	case ERR_CHANOPRIVSNEEDED:
	{
		str_send += " " + _nickname + " " + arg[0] + " :You're not channel operator";
		break;
	}
	case ERR_NOSUCHNICK:
	{
		str_send += " " + _nickname + " " + arg[0] + " :No such nick/channel";
		break;
	}
	case ERR_USERSDONTMATCH:
	{
		str_send += " " + _nickname + " :Cant change mode for other users";
		break;
	}
	case ERR_UMODEUNKNOWNFLAG:
	{
		str_send += " " + _nickname + " :Unknown MODE flag";
		break;
	}
	case ERR_NOOPERHOST:
	{
		str_send += " " + _nickname + " :No O-lines for your host";
		break;
	}
	case RPL_YOUREOPER:
	{
		str_send += " " + _nickname + " :You are now an IRC operator";
		break;
	}
	case RPL_REHASHING:
	{
		str_send += " " + _nickname + " " + arg[0] + " :Rehashing";
		break;
	}
	case RPL_BADREHASH:
	{
		str_send += " " + _nickname + " " + arg[0] + " :Bad rehash error in file return to previous settings";
		break;
	}
	case ERR_NOPRIVILEGES:
	{
		str_send += " " + _nickname + " :Permission Denied- You're not an IRC operator";
		break;
	}
	case ERR_NOSUCHSERVER:
	{
		str_send += " " + _nickname + " " + arg[0] + " :No such server";
		break;
	}
	default:
		str_send += " " + _nickname + " :Unknown error";
		break;
	}
	sendMessageToClient(str_send);
	return (code);
}

