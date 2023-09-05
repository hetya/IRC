#include "Irc.class.hpp"

int	receiveFromClient(Irc &irc, struct pollfd fd, Client &client) {
    // Check that client is still connected
    if (!client.willBeKill()) {
        int nChars; // Indique le nombre de caractères qui a été reçu ou envoyé
        char buffer[BUFFER_SIZE];

        nChars = recv(fd.fd, buffer, BUFFER_SIZE - 1, 0);
        // EOF received or error while receiving chars, closing connection
        if (nChars <= 0) {
            return -1;
        }
        // Something was received
        else {
            buffer[nChars]=0;
            client.add_to_buffRead(buffer);
            process_line(irc, client);
        }
    }
    return 0;
}

int	sendToClient(Irc &irc, struct pollfd fd, Client &client) {
    (void)irc;
    if (client.isThereSomethingToSend()) {
        std::string buf = client.getBuffWrite();
        int rc = send(fd.fd, buf.c_str(), buf.size(), 0);

        // EOF sent or error while sending chars, closing connection
        if (rc <= 0) {
            return -1;
        }
        // Something was written, remove from client _writeBuff
        else {
            client.setBuffWrite(buf.substr(rc, std::string::npos));
        }
    }
    return 0;
}
