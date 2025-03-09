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
#include "cmd.h"  // Include the cmd.h header for command constants

void *keyboard_input_handler(void *arg) {
    Session *session = (Session *)arg;
    char input[1024];
    
    log_message(INFO, "Keyboard input handler started");
    
    while (1) {
        memset(input, 0, sizeof(input));
        
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

        uint8_t command = 0x10;
        
        if (strncmp(input, "/login", 6) == 0) {
            command = LOGIN;
            // Extract username (skip "/login ")
            char *username = input + 7;
            
            Message *msg = message_create(command);
            if (msg == NULL) {
                log_message(ERROR, "Failed to create message");
                continue;
            }
            
            // Write username to message
            message_write(msg, username, strlen(username) + 1);
            log_message(INFO, "Sending login message");
            session->sendMessage(session, msg);
            message_destroy(msg);
        }
        else if (strncmp(input, "/register", 9) == 0) {
            command = REGISTER;
            // Extract username (skip "/register ")
            char *username = input + 10;
            
            Message *msg = message_create(command);
            if (msg == NULL) {
                log_message(ERROR, "Failed to create message");
                continue;
            }
            
            // Write username to message
            message_write(msg, username, strlen(username) + 1);
            
            session->sendMessage(session, msg);
            message_destroy(msg);
        }
        else if (strcmp(input, "/logout") == 0) {
            command = LOGOUT;
            
            Message *msg = message_create(command);
            if (msg == NULL) {
                log_message(ERROR, "Failed to create message");
                continue;
            }
            
            session->sendMessage(session, msg);
            message_destroy(msg);
        }
        else {
            // Regular text message
            // You might want to add a TEXT_MESSAGE constant to cmd.h
            #define TEXT_MESSAGE 0x10  // Add this to cmd.h
            
            Message *msg = message_create(TEXT_MESSAGE);
            if (msg == NULL) {
                log_message(ERROR, "Failed to create message");
                continue;
            }
            
            // Write the full text message
            message_write(msg, input, strlen(input) + 1);
            
            session->sendMessage(session, msg);
            message_destroy(msg);
        }
    }
    
    log_message(INFO, "Keyboard input handler exiting");
    return NULL;
}

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

    pthread_t keyboard_thread;
    if (pthread_create(&keyboard_thread, NULL, keyboard_input_handler, session) != 0) {
        log_message(ERROR, "Failed to create keyboard input thread");
        return 1;
    }
    
    pthread_join(keyboard_thread, NULL);
    
    session->close(session);
    destroySession(session);
    
    log_message(INFO, "Application exiting");
    return 0;
}