#include <stdio.h>
#include <stdlib.h>
#include "session.h"
#include "log.h"
#include <signal.h>


int main(int argc, char *argv[]) {
    Session *session = createSession();
    if (session == NULL) {
        log_message(ERROR, "Failed to create session");
        return 1;
    }
    signal(SIGPIPE, SIG_IGN);

    session->connect(session, "127.0.0.1", 1609);

    //open thread to receive commands, message from keyboard
}
