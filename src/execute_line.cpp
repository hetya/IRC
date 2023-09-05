#include <iostream>
#include "Client.class.hpp"
#include "Irc.class.hpp"
#include <map>
#include <vector>

void process_line(Irc &irc, Client &client)
{
	while (irc.get_client_index(client) != irc.get_clients().max_size() && client.line_in_buffRead())
	{
		std::vector<std::string> splittedLine = split(client.get_line());

		// Empty command
		if (splittedLine.size() == 0) {
			std::cout << client.get_nickname() << " executed an empty command" << std::endl;
			return ;
		}
	
		// If user is registered, search over all commands and execute it if found, return not found otherwise
		if (client.is_register()) {
			if (irc.commands.find(splittedLine[0]) != irc.commands.end()) {
				std::cout << client.get_nickname() << " executed " << splittedLine[0] << std::endl;
				irc.commands.find(splittedLine[0])->second(client , splittedLine);
			}
			else
				std::cout << client.get_nickname() << " executed an unknown command" << std::endl;
		} else {
			// Find command in unregistered commands. If not found, search in all commands
			if (irc.unregisteredCommands.find(splittedLine[0]) != irc.unregisteredCommands.end()) {
				std::cout << "Someone is registrating, executed " << splittedLine[0] << std::endl;
				irc.unregisteredCommands.find(splittedLine[0])->second(client , splittedLine);
			}
			else {
				if (irc.commands.find(splittedLine[0]) != irc.commands.end()) {
					std::cout << "Someone executed an existing command but is not registered" << std::endl;
					client.send_code(ERR_NOTREGISTERED);
				}
				else
					std::cout << "Someone executed an unknown command and is not registered" << std::endl;
			}
		}
	}
}