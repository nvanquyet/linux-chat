#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "session.h"
#include "log.h"
#include <signal.h>
#include <unistd.h>
#include "message.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "session.h"
#include "log.h"
#include <signal.h>
#include <unistd.h>
#include "message.h"
#include "cmd.h"

void *keyboard_input_handler(void *arg) {
    Session *session = (Session *)arg;
    char input[1024];
    
    while (session->isRunning) {
        if (!session->connected) {
            log_message(INFO, "Server disconnected, keyboard handler exiting");
            break;
        }
        
        memset(input, 0, sizeof(input));
        
        fd_set readfds;
        struct timeval tv;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        
        int select_result = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);
        
        if (select_result == -1) {
            log_message(ERROR, "Select error on stdin");
            break;
        } else if (select_result == 0) {
            continue;
        }
        
        if (fgets(input, sizeof(input) - 1, stdin) == NULL) {
            log_message(ERROR, "Error reading from stdin");
            break;
        }
        
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }
        
        if (strcmp(input, "/exit") == 0 || strcmp(input, "/quit") == 0) {
            log_message(INFO, "Exit command received");
            break;
        }
        

        if (!session->connected) {
            log_message(INFO, "Server disconnected, keyboard handler exiting");
            break;
        }

        uint8_t command = 0x10;
        
        if (strncmp(input, "/login", 6) == 0) {
            //login
            log_message(INFO, "Login command received");
        }
    }
    
    log_message(INFO, "Keyboard input handler exiting");
    return NULL;
}

// First, modify your main function to check connection status
int main(int argc, char *argv[])
{
    Session *session = createSession();
    if (session == NULL)
    {
        log_message(ERROR, "Failed to create session");
        return 1;
    }
    signal(SIGPIPE, SIG_IGN);


    session->connect(session, "127.0.0.1", 1609);
 
    if (!session->connected) {
        log_message(ERROR, "Connection failed, exiting");
        destroySession(session);
        return 1;
    }
    log_message(INFO, "Connected to server, starting keyboard input handler");

    pthread_t keyboard_thread;
    if (pthread_create(&keyboard_thread, NULL, keyboard_input_handler, session) != 0) {
        log_message(ERROR, "Failed to create keyboard input thread");
        session->close(session);
        destroySession(session);
        return 1;
    }
    
    pthread_join(keyboard_thread, NULL);
    
    session->close(session);
    destroySession(session);
    
    log_message(INFO, "Application exiting");
    return 0;
}