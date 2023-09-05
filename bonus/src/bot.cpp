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
#include <iostream>

typedef struct s_bot
{
	int						socket;
	struct sockaddr_in		irc_serv_address;
	std::string				serv_ip;
	int						serv_port;
	bool					isregister;
	std::string 			password;
	std::string 			nickname;
	std::string				buff;
	bool					run;
} t_bot;

std::vector<std::string>	split(std::string const &s, std::string sep = " ") {
	size_t start = s.find_first_not_of(sep);
    size_t end = s.find(sep, start);
	std::vector<std::string> splittedLine;

    while (end != std::string::npos) {
		splittedLine.push_back(s.substr(start, end - start));
        start = s.find_first_not_of(sep, end);
        end = s.find(sep, start);
    }
	if (start != end)
		splittedLine.push_back(s.substr(start, end - start));
	return splittedLine;
}

bool serv_connect(t_bot &bot)
{
	bot.socket = socket(AF_INET, SOCK_STREAM, 0);
	if (bot.socket == -1)
	{
		std::cerr << "irc_bot: Socket can't be created" << std::endl;
		return (1);
	}
	memset(&bot.irc_serv_address, 0, sizeof(bot.irc_serv_address));
	bot.irc_serv_address.sin_family = AF_INET;
	bot.irc_serv_address.sin_port = htons(bot.serv_port); // check si le port n'est pas negatif et qu'il soit complet
	if(inet_pton(AF_INET, bot.serv_ip.c_str(), &bot.irc_serv_address.sin_addr) <= 0) // check address ip
	{
		std::cerr << "irc_bot: Inet_pton error" << std::endl;
		return (0);
	}
	if(connect(bot.socket, (struct sockaddr *)&bot.irc_serv_address, sizeof(bot.irc_serv_address)) < 0)
	{
		bot.socket = 0;
		std::cerr << "irc_bot: Connect Failed" << std::endl;
		return (0);
	}
	std::cout << "Connected" << std::endl;
	return (1);
}

bool	sendMessageToServer(t_bot bot, std::string const &str) {
	if (send(bot.socket, (str + "\r\n").c_str(), str.size() + 2, 0) == -1) {
        std::cerr << "irc_bot: error while sending message to server" << std::endl;
        return (1);
    }
	return (0);
}

bool	bot_registration(t_bot &bot)
{
	sendMessageToServer(bot, "PASS " + bot.password);
	sendMessageToServer(bot, "NICK " + bot.nickname);
	sendMessageToServer(bot, "USER bot 0 * IrcBot");
	return (1);
}

bool	check_registration(t_bot &bot, std::string str)
{
	if (split(str)[0] == "001")
	{
		bot.isregister = 1;
		std::cout << "registered"  << std::endl;
		return (1);
	}
	return (0);
}

void	add_buff(t_bot &bot)
{
	int nb_character_recv;
	char buffer[1024];

	// nb_character_recv=recv(bot.socket, buffer, 1023, MSG_DONTWAIT);
	nb_character_recv=recv(bot.socket, buffer, 1515, 0);
	if (nb_character_recv <= 0 )
		bot.run = false;
	if (nb_character_recv != -1)
	{
		buffer[nb_character_recv]=0;
		bot.buff += buffer;
	}
}

bool	line_in_buff(t_bot &bot) {
	return (bot.buff.find('\n') != std::string::npos);
}

std::string	get_line(t_bot &bot)
{
	std::string	line;
	size_t rn;

	if ((rn = bot.buff.find("\r\n")) == std::string::npos)
		return ("");
	line = bot.buff.substr(0, rn);
	bot.buff.erase(0, rn + 2);
	return (line);
}

void say_hello(t_bot &bot, std::string str)
{
	std::vector<std::string> token;

	token = split(str);
	if (token.size() == 4 && (token[1] == "PRIVMSG" || token[1] == "NOTICE") && (token[3] == ":Hello" || token[3] == "::Hello"))
	{
		std::cout << "SayHello: send: " << "NOTICE " + token[0].substr(1 , token[0].find('!') - 1) + " Hello " + token[0].substr(1 , token[0].find('!') - 1) + "!" << std::endl;
		sendMessageToServer(bot, "NOTICE " + token[0].substr(1 , token[0].find('!') - 1) + " Hello " + token[0].substr(1 , token[0].find('!') - 1) + "!");
	}
}

bool	exec_line(t_bot &bot)
{
	std::string	line;

	while (bot.run)
	{
		if (!bot.isregister)
			bot_registration(bot);
		add_buff(bot);
		while (line_in_buff(bot))
		{
			line = get_line(bot);
			if (!bot.isregister)
			{
				if (!check_registration(bot, line))
				{
					return (0);
				}
				else
				{
					line = get_line(bot);
					line = get_line(bot);
					line = get_line(bot);
					line = get_line(bot);
					continue ;
				}
			}
			// gestion ligne
			say_hello(bot, line);
			add_buff(bot);
		}
	}
	return (1);
}

int main(int argc, char **argv)
{
	t_bot bot;

    if (argc != 5)
    {
        std::cerr << "irc_bot: Wrong numbers of arguments. please use ./bot <ipAddress> <port> <name> <password>" << std::endl;
        return (1);
    }
	bot.run = true;
	bot.isregister = false;
	bot.serv_ip = argv[1];
	bot.serv_port = atoi(argv[2]);
	bot.nickname = argv[3];
	bot.password = argv[4];
	if (!serv_connect(bot))
		return (1);
	if (!exec_line(bot))
		return (1);
	close(bot.socket);
}