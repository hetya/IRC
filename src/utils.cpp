#include "Irc.class.hpp"
#include <stdlib.h>
#include <cmath>

bool	check_args(std::string port, std::string password)
{
	// checking the port
	unsigned int port_nb;

	if (port.length() > 5)
	{
		std::cerr << "Error: not a port" << std::endl;
		return (0);
	}
	for (size_t i = 0; i < port.length(); i++)
	{
		if (!std::isdigit(port[i]))
		{
			std::cerr << "Error: letter in port" << std::endl;
			return (0);
		}
	}
	port_nb = atoi(port.c_str());
	if (port_nb < 1 || port_nb > 65535)
	{
		std::cerr << "Error: port not in port range" << std::endl;
		return (0);
	}
	// checking the password
	for (size_t i = 0; i < password.length(); i++)
	{
		if (!std::isprint(password[i]))
		{
			std::cerr << "Error: not printable char in password" << std::endl;
			return (0);
		}
	}
	return (1);
}

void	ircServerErrorsHandling(IrcErrors err, struct pollfd *fds) {
	std::string	error;

	if (fds != NULL) {
		for (size_t i = 0; i < MAX_CLIENTS; i++) {
			if (fds[i].fd > 0) {
				close(fds[i].fd);
			}
		}
	}

	switch (err)
	{
	case NumberOfArgs:
		error = "Wrong numbers of arguments. please use ./ircserv <port> <password>";
		break;
	case Listen:
		error = "Error while listening to client";
		break;
	case SocketError:
		error = "Error while creating the socket";
		break;
	case PollError:
		error = "Error while calling poll";
		break;
	case PollTimeoutError:
		error = "Timeout reached when calling poll";
		break;

	default:
		error = "Encountered an error";
		break;
	}
	std::cerr << "ircserv: " << error << std::endl;
	exit(EXIT_FAILURE);
}

int count_digit(int nb)
{
	if (nb < 0)
		return (0);
	if (nb == 0)
		return (1);
	return (static_cast<int>(log10(abs(nb))) + 1);
}

std::vector<std::string>	split(std::string const &s, std::string sep) {
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
std::string	vecStringJoin(std::vector<std::string>::const_iterator start, std::vector<std::string>::const_iterator end) {
	std::string	str;
	for (std::vector<std::string>::const_iterator it = start; it != end; ++it) {
		str += *it;
		if (it != end - 1)
			str += " ";
	}
	return str;
}

std::string to_string(int val) {
    char buf[25];
    int got_len = snprintf(buf, sizeof(buf), "%d", val);

    if (got_len > static_cast<int>(sizeof(buf)))
        throw std::invalid_argument("integer is longer than string buffer");

    return std::string(buf);
}