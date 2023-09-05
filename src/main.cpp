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
#include <ctime>
#include "Irc.class.hpp"

// https://modern.ircdocs.horse/

int    main(int argc, char *argv[])
{
    int                         serverFD, clientFD;
    struct    sockaddr_in       serverAddress, peerAddress;
    socklen_t                   peerAddressSize;
    struct    pollfd            fds[MAX_CLIENTS];
    int                         nfds = 1;

    // Number of args
    if (argc != 3)
        ircServerErrorsHandling(NumberOfArgs, NULL);

    if (!check_args(argv[1], argv[2])) {
        return (1);
    }
    Irc    irc(argv[1], argv[2]);
    if (irc.parse_file() != 0)
    {
        std::cerr << "Error: while parsing IRCd file" << std::endl;
        return (1);
    }
    irc.set_serverStat(OK_SERV_STAT);
    std::vector< Client > &clients = irc.get_clients();
    clients.reserve(MAX_CLIENTS);
    serverFD = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFD == -1)
        ircServerErrorsHandling(SocketError, NULL);

    fcntl(serverFD, F_SETFL, O_NONBLOCK);
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(atoi(argv[1]));
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
                    char ipd_client[INET_ADDRSTRLEN];
                inet_ntop( AF_INET, &serverAddress.sin_addr, ipd_client, INET_ADDRSTRLEN );
    memset(&peerAddress, 0, sizeof(peerAddress));
    peerAddress.sin_family = AF_INET;
    peerAddress.sin_port = htons(atoi(argv[1]));
    peerAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    memset(fds, 0 , sizeof(fds));
    fds[0].fd = serverFD;
    fds[0].events = POLLIN | POLLOUT;

    // Fix already bind error
    int    yes = 1;
    if (setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        perror("ircserv");
        exit(EXIT_FAILURE);
    }

    if (bind(serverFD, (struct sockaddr *) &serverAddress,sizeof(serverAddress)) == -1)
    {
        std::cerr << "ERR_BIND: " << inet_ntoa(serverAddress.sin_addr) << ":" << serverAddress.sin_port << std::endl;
        return (1);
    }

    if (listen(serverFD, LISTEN_BACKLOG) < 0)
        ircServerErrorsHandling(Listen, NULL);

    while (irc.get_serverStat() == 0)
    {
        int poll_response = poll(fds, nfds, 1000 * 60 * 3);

        if (poll_response < 0) {
            ircServerErrorsHandling(PollError, fds);
        }
        else if (poll_response == 0) {
            ircServerErrorsHandling(PollTimeoutError, fds);
        }
        int current_size = nfds;

        for (int i = 0; i < current_size; i++) {

            // No event happening on this fd
            if (fds[i].revents == 0) {
                continue;
            }

            // Accepting connections
            if (fds[i].fd == serverFD) {
                if (fds[i].revents != POLLIN)
                    continue;
                do
                {
                    peerAddressSize= 16;
                    clientFD = accept(serverFD, (struct sockaddr *) &peerAddress, &peerAddressSize);
                    if (clientFD < 0) {
                        break;
                    }
                    std::cout << "New client joined!" << std::endl;
                    
                    struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&peerAddress;
                    struct in_addr ipAddr = pV4Addr->sin_addr;
                    char ip_client[INET_ADDRSTRLEN];
                    inet_ntop( AF_INET, &ipAddr, ip_client, INET_ADDRSTRLEN );

                    clients.push_back(Client(&irc, ip_client));

                    fds[nfds].fd = clientFD;
                    fds[nfds].events = POLLIN | POLLOUT;
                    fds[nfds].revents = 0;
                    nfds++;
                } while (clientFD != -1);
            }
            // Polling existing connections
            else {
                bool close_conn = false;

				switch(fds[i].revents)
				{
                case (POLLIN | POLLOUT) :
                    // If receiveFromClient or sendToClient returns < 0, disconnects client from server
                    if (receiveFromClient(irc, fds[i], clients[i - 1]) < 0
                        or sendToClient(irc, fds[i], clients[i - 1]) < 0) {
                        close_conn = true;
                    }
                break;

                case POLLIN :
                    // If receiveFromClient returns < 0, disconnects client from server
                    if (receiveFromClient(irc, fds[i], clients[i - 1]) < 0) {
                        close_conn = true;
                    }
                break;

                case POLLOUT :
                    // If sendToClient returns < 0, disconnects client from server
                    if (sendToClient(irc, fds[i], clients[i - 1]) < 0) {
                        close_conn = true;
                    }
                    break;
                // Client wants to disconnect
                case POLLHUP | POLLIN :
                    close_conn = true;
                    break;
                default :
                    close_conn = true;
				}

                // A client disconnects
                if (close_conn
                    || (clients[i - 1].willBeKill() && !clients[i - 1].isThereSomethingToSend())
                    || irc.get_serverStat() != 0) {
					irc.sendMessageToClients( ":" +  clients[i - 1].get_nickname() + " QUIT " + ":Quit:" + clients[i - 1].exitraison);

                    // Erase client from clients
                    clients.erase( clients.begin() + (i - 1));


                    // Erase client from fds
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    for (int k = 0; k < nfds; k++) {
                        if (fds[k].fd == -1) {
                            for(int j = k; j < nfds; j++) {
                                fds[j].fd = fds[j + 1].fd;
                            }
                            k--;
                            nfds--;
                        }
                    }
                    break;
                }
            }
        }
    }
    close(serverFD);
}