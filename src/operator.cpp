#include "Irc.class.hpp"

int oper(Client & client, std::vector<std::string> args)
{
	if (args.size() < 3)
		return (client.send_code(ERR_NEEDMOREPARAMS, args[0]));
	if (args[1] != client.irc->get_operLogin()[1])
		return (0); // bad login
	if (args[2] != client.irc->get_operLogin()[2])
		return (client.send_code(ERR_PASSWDMISMATCH));
	if (client.get_ip() != client.irc->get_operLogin()[0])
		return (client.send_code(ERR_NOOPERHOST));
	client.setop();
	client.send_code(RPL_YOUREOPER);
	client.sendMessageToClient(":" + client.get_nickname() + " MODE " + client.get_nickname() + " +o");
	return (0);
}

int op_kill(Client & client, std::vector<std::string> args)
{
	Client  *client_to_kill;

	if (!client.is_op())
		return (client.send_code(ERR_NOPRIVILEGES));
	if (args.size() < 3)
		return (client.send_code(ERR_NEEDMOREPARAMS, args[0]));
	if((client_to_kill = client.irc->get_client_by_nickname(args[1])) == NULL)
		return (0);// error client doesn't exit
	client.exitraison = args[2];
	client_to_kill->set_kill();
	return (0);
}

int op_rehash(Client & client, std::vector<std::string> args)
{
	std::vector<std::string>	backupLogOper;

	(void) args;
	if (!client.is_op())
		return (client.send_code(ERR_NOPRIVILEGES));
	backupLogOper = client.irc->get_operLogin();

	client.irc->empty_operLogin();
	if (client.irc->parse_file() != 0)
	{
		std::cerr << "now returning to prevous IRCd settings" << std::endl;
		client.irc->set_operLoginByVec(backupLogOper);
		client.send_code(RPL_BADREHASH, "IRCd");
		return (1);
	}
	client.send_code(RPL_REHASHING, "IRCd");
	
	return (0);
}

int op_squit(Client & client, std::vector<std::string> args)
{
	if (!client.is_op())
		return (client.send_code(ERR_NOPRIVILEGES));
	if (args.size() < 3)
		return (client.send_code(ERR_NEEDMOREPARAMS, args[0]));
	if (args[1] != SERVERNAME)
		return (client.send_code(ERR_NOSUCHSERVER, args[1]));

	client.irc->set_serverStat(QUIT_SERV_STAT);
	return (0);
}
