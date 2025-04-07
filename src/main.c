#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "session.h"
#include "user.h"
#include "group.h"
#include "log.h"
#include <signal.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include "message.h"

// Command definitions
#define CMD_EXIT        "/exit"
#define CMD_QUIT        "/quit"
#define CMD_LOGIN       "/login"
#define CMD_LOGOUT      "/logout"
#define CMD_REGISTER    "/register"
#define CMD_USERS       "/users"
#define CMD_GROUPS      "/groups"
#define CMD_JOIN_GROUP  "/join"
#define CMD_LEAVE_GROUP "/leave"
#define CMD_MESSAGE     "/msg"
#define CMD_USER_MSG    "/u-msg"
#define CMD_CREATE_GROUP "/create"
#define CMD_DELETE_GROUP "/delete"
#define CMD_HISTORY     "/history"
#define CMD_HELP        "/help"
#define CMD_PROFILE     "/profile"
#define CMD_STATUS      "/status"

// Function to display help information
void display_help() {
    printf("\n----- Chat Client Commands -----\n");
    printf("%s or %s - Exit the application\n", CMD_EXIT, CMD_QUIT);
    printf("%s <username> <password> - Login to the server\n", CMD_LOGIN);
    printf("%s - Logout from the server\n", CMD_LOGOUT);
    printf("%s <username> <password> - Register a new account\n", CMD_REGISTER);
    printf("%s - List all online users\n", CMD_USERS);
    printf("%s - List all available groups\n", CMD_GROUPS);
    printf("%s <group_id> - Join a group\n", CMD_JOIN_GROUP);
    printf("%s <group_id> - Leave a group\n", CMD_LEAVE_GROUP);
    printf("%s <group_id> <message> - Send message to a group\n", CMD_MESSAGE);
    printf("%s <user_id> <message> - Send direct message to a user\n", CMD_USER_MSG);
    printf("%s <group_name> - Create a new group\n", CMD_CREATE_GROUP);
    printf("%s <group_id> - Delete a group\n", CMD_DELETE_GROUP);
    printf("%s - Show your profile information\n", CMD_PROFILE);
    printf("%s - Show connection status\n", CMD_STATUS);
    printf("%s - Display this help message\n", CMD_HELP);
    printf("---------------------------------\n\n");
}

// Function to safely check if the service and user are valid
int check_session_valid(Session *session, const char *command_name) {
    if (session == NULL) {
        log_message(ERROR, "Session is NULL when executing %s", command_name);
        return 0;
    }

    if (!session->connected) {
        log_message(ERROR, "Not connected to server. Cannot execute %s", command_name);
        return 0;
    }

    if (session->service == NULL) {
        log_message(ERROR, "Service is NULL. Cannot execute %s", command_name);
        return 0;
    }

    if (!session->isLogin || session->user == NULL) {
        log_message(ERROR, "You must be logged in to use %s", command_name);
        return 0;
    }

    return 1;
}

// Function to handle keyboard input
void *keyboard_input_handler(void *arg) {
    Session *session = (Session *)arg;
    char input[1024];

    // Display help information at startup
    display_help();

    while (session->isRunning) {
        if (!session->connected) {
            log_message(INFO, "Server disconnected, keyboard handler exiting");
            break;
        }

        memset(input, 0, sizeof(input));

        // Set up select to allow for non-blocking input
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

        // Read input from stdin
        if (fgets(input, sizeof(input) - 1, stdin) == NULL) {
            log_message(ERROR, "Error reading from stdin");
            break;
        }

        // Remove trailing newline
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }

        // Check for exit command
        if (strcmp(input, CMD_EXIT) == 0 || strcmp(input, CMD_QUIT) == 0) {
            log_message(INFO, "Exit command received");
            break;
        }

        // Check connection status
        if (!session->connected) {
            log_message(INFO, "Server disconnected, keyboard handler exiting");
            break;
        }

        // Handle help command
        if (strcmp(input, CMD_HELP) == 0) {
            display_help();
            continue;
        }

        // Handle login command
        if (strncmp(input, CMD_LOGIN, strlen(CMD_LOGIN)) == 0) {
            char *cmd_ptr = input + strlen(CMD_LOGIN);

            // Skip whitespace
            while (*cmd_ptr == ' ') cmd_ptr++;

            if (*cmd_ptr == '\0') {
                log_message(ERROR, "Usage: /login <username> <password>");
                continue;
            }

            char username[256] = {0};
            char password[256] = {0};

            char *space_ptr = strchr(cmd_ptr, ' ');
            if (space_ptr == NULL) {
                log_message(ERROR, "Usage: /login <username> <password>");
                continue;
            }

            // Copy username
            size_t username_len = space_ptr - cmd_ptr;
            if (username_len >= sizeof(username)) {
                username_len = sizeof(username) - 1;
            }
            strncpy(username, cmd_ptr, username_len);
            username[username_len] = '\0';

            // Skip whitespace after username
            cmd_ptr = space_ptr + 1;
            while (*cmd_ptr == ' ') cmd_ptr++;

            // Copy password
            strncpy(password, cmd_ptr, sizeof(password) - 1);

            if (strlen(username) == 0 || strlen(password) == 0) {
                log_message(ERROR, "Usage: /login <username> <password>");
                continue;
            }

            if (session->isLogin) {
                log_message(ERROR, "Already logged in. Use /logout first");
                continue;
            }

            if (session->service == NULL) {
                log_message(ERROR, "Service unavailable");
                continue;
            }

            if (session->user != NULL) {
                destroyUser(session->user);
                session->user = NULL;
            }

            User *user = createUser(NULL, session, username, password);
            if (user == NULL) {
                log_message(ERROR, "Failed to create user");
                continue;
            }

            session->user = user;
            user->session = session;

            user->login(user);
        }

        // Handle logout command
        else if (strcmp(input, CMD_LOGOUT) == 0) {
            if (session->user != NULL && session->isLogin) {
                session->user->logout(session->user);
                session->isLogin = false;
            } else {
                log_message(ERROR, "Not logged in");
            }
        }

        // Handle register command
        else if (strncmp(input, CMD_REGISTER, strlen(CMD_REGISTER)) == 0) {
            char *cmd_ptr = input + strlen(CMD_REGISTER);

            // Skip whitespace
            while (*cmd_ptr == ' ') cmd_ptr++;

            if (*cmd_ptr == '\0') {
                log_message(ERROR, "Usage: /register <username> <password>");
                continue;
            }

            char username[256] = {0};
            char password[256] = {0};

            char *space_ptr = strchr(cmd_ptr, ' ');
            if (space_ptr == NULL) {
                log_message(ERROR, "Usage: /register <username> <password>");
                continue;
            }

            // Copy username
            size_t username_len = space_ptr - cmd_ptr;
            if (username_len >= sizeof(username)) {
                username_len = sizeof(username) - 1;
            }
            strncpy(username, cmd_ptr, username_len);
            username[username_len] = '\0';

            // Skip whitespace after username
            cmd_ptr = space_ptr + 1;
            while (*cmd_ptr == ' ') cmd_ptr++;

            // Copy password
            strncpy(password, cmd_ptr, sizeof(password) - 1);

            if (strlen(username) == 0 || strlen(password) == 0) {
                log_message(ERROR, "Usage: /register <username> <password>");
                continue;
            }

            if (session->service != NULL) {
                User *user = createUser(NULL, session, username, password);
                if (user == NULL) {
                    log_message(ERROR, "Failed to create user");
                    continue;
                }
                session->user = user;
                user->session = session;
                user->userRegister(user);
            } else {
                log_message(ERROR, "Service is NULL");
            }
        }

        // Handle users command
        else if (strcmp(input, CMD_USERS) == 0) {
            if (check_session_valid(session, "users command")) {
                session->service->get_online_users(session->service);
            }
        }
        // Handle groups command
        else if (strcmp(input, CMD_GROUPS) == 0) {
            if (check_session_valid(session, "groups command")) {
                session->service->get_group_list(session->service, session->user);
            }
        }
        // Handle join group command
        else if (strncmp(input, CMD_JOIN_GROUP, strlen(CMD_JOIN_GROUP)) == 0) {
            char *cmd_ptr = input + strlen(CMD_JOIN_GROUP);

            // Skip whitespace
            while (*cmd_ptr == ' ') cmd_ptr++;

            if (*cmd_ptr == '\0') {
                log_message(ERROR, "Usage: /join <group_id>");
                continue;
            }

            int group_id = atoi(cmd_ptr);
            if (group_id <= 0) {
                log_message(ERROR, "Invalid group ID: %s", cmd_ptr);
                continue;
            }

            if (check_session_valid(session, "join command")) {
                session->service->join_group(session->service, session->user, group_id);
            }
        }
        // Handle leave group command
        else if (strncmp(input, CMD_LEAVE_GROUP, strlen(CMD_LEAVE_GROUP)) == 0) {
            char *cmd_ptr = input + strlen(CMD_LEAVE_GROUP);

            // Skip whitespace
            while (*cmd_ptr == ' ') cmd_ptr++;

            if (*cmd_ptr == '\0') {
                log_message(ERROR, "Usage: /leave <group_id>");
                continue;
            }

            int group_id = atoi(cmd_ptr);
            if (group_id <= 0) {
                log_message(ERROR, "Invalid group ID: %s", cmd_ptr);
                continue;
            }

            if (check_session_valid(session, "leave command")) {
                session->service->leave_group(session->service, session->user, group_id);
            }
        }
        // Handle create group command
        else if (strncmp(input, CMD_CREATE_GROUP, strlen(CMD_CREATE_GROUP)) == 0) {
            // Skip whitespace
            char *cmd_ptr = input + strlen(CMD_CREATE_GROUP) + 1;
            //while (*cmd_ptr == ' ') cmd_ptr++;
            //
            if (*cmd_ptr == '\0') {
                log_message(ERROR, "Usage: /create <group_name>");
                continue;
            }

            char group_name[256] = {0};
            strncpy(group_name, cmd_ptr, sizeof(group_name) - 1);

            if (check_session_valid(session, "create command")) {
                // Check if create_group function exists in the service structure
                if (session->service->create_group) {
                    session->service->create_group(session->service, session->user, group_name);
                    log_message(INFO, "Group creation request sent for '%s'", group_name);
                } else {
                    log_message(ERROR, "Create group function is not implemented in this version");
                }
            }
        }
        else if (strncmp(input, CMD_DELETE_GROUP, strlen(CMD_DELETE_GROUP)) == 0) {
            // Skip whitespace
            char *cmd_ptr = input + strlen(CMD_DELETE_GROUP) + 1;

            if (*cmd_ptr == '\0') {
                log_message(ERROR, "Usage: /delete <group_id>");
                continue;
            }
            int group_id = atoi(cmd_ptr);
            if (group_id <= 0) {
                log_message(ERROR, "Invalid group ID: %s", cmd_ptr);
                continue;
            }
            if (check_session_valid(session, "delete command")) {
                // Kiểm tra xem service có hỗ trợ hàm delete_group không
                if (session->service->delete_group) {
                    session->service->delete_group(session->service, session->user, group_id);
                    log_message(INFO, "Group delete request sent for ID %d", group_id);
                } else {
                    log_message(ERROR, "Service does not support group deletion");
                }
            }
        } // Handle delete group command


        // Handle group message command
        else if (strncmp(input, CMD_MESSAGE, strlen(CMD_MESSAGE)) == 0) {
            char *cmd_ptr = input + strlen(CMD_MESSAGE) + 1;

            // Skip whitespace
            //while (*cmd_ptr == ' ') cmd_ptr++;

            if (*cmd_ptr == '\0') {
                log_message(ERROR, "Usage: /msg <group_id> <message>");
                continue;
            }

            char *group_id_str = cmd_ptr;
            char *message = strchr(cmd_ptr, ' ');

            if (message == NULL) {
                log_message(ERROR, "Usage: /msg <group_id> <message>");
                continue;
            }

            // Replace space with null terminator to separate group_id and message
            *message = '\0';
            message++;  // Move to the start of the message

            // Skip whitespace at the beginning of message
            while (*message == ' ') message++;

            int group_id = atoi(group_id_str);
            if (group_id <= 0) {
                log_message(ERROR, "Invalid group ID: %s", group_id_str);
                continue;
            }

            if (strlen(message) == 0) {
                log_message(ERROR, "Message cannot be empty");
                continue;
            }

            if (check_session_valid(session, "message command")) {
                session->service->send_group_message(session->service, session->user, group_id, message);
            }
        }

        // Handle direct user message command
        else if (strncmp(input, CMD_USER_MSG, strlen(CMD_USER_MSG)) == 0) {
            char *cmd_ptr = input + strlen(CMD_USER_MSG) + 1;
            // Skip whitespace
            //while (*cmd_ptr == ' ') cmd_ptr++;

            if (*cmd_ptr == '\0') {
                log_message(ERROR, "Usage: /u-msg <user_id> <message>");
                continue;
            }

            char *user_id_str = cmd_ptr;
            char *message = strchr(cmd_ptr, ' ');

            if (message == NULL) {
                log_message(ERROR, "Usage: /u-msg <user_id> <message>");
                continue;
            }

            // Replace space with null terminator to separate user_id and message
            *message = '\0';
            message++;  // Move to the start of the message

            // Skip whitespace at the beginning of message
            while (*message == ' ') message++;

            int user_id = atoi(user_id_str);
            if (user_id <= 0) {
                log_message(ERROR, "Invalid user ID: %s", user_id_str);
                continue;
            }

            if (strlen(message) == 0) {
                log_message(ERROR, "Message cannot be empty");
                continue;
            }

            if (check_session_valid(session, "direct message command")) {
                // Check if send_direct_message function exists in the service structure
                if (session->service->send_user_message) {
                    session->service->send_user_message(session->service, session->user, user_id, message);
                    log_message(INFO, "Direct message sent to user %d", user_id);
                } else {
                    log_message(ERROR, "Direct messaging is not implemented in this version");
                }
            }
        }

        // Handle profile command
        else if (strcmp(input, CMD_PROFILE) == 0) {
            if (session->user != NULL && session->isLogin) {
                printf("\n----- Your Profile -----\n");
                printf("Username: %s\n", session->user->username);
                printf("User ID: %d\n", session->user->id);
                printf("Status: %s\n", session->isLogin ? "Online" : "Offline");
                printf("------------------------\n\n");
            } else {
                log_message(ERROR, "Not logged in");
            }
        }

        // Handle status command
        else if (strcmp(input, CMD_STATUS) == 0) {
            printf("\n----- Connection Status -----\n");
            printf("Connected to server: %s\n", session->connected ? "Yes" : "No");
            printf("Logged in: %s\n", session->isLogin ? "Yes" : "No");
            if (session->isLogin && session->user != NULL) {
                printf("Username: %s\n", session->user->username);
            }
            printf("-----------------------------\n\n");
        } else if (strcmp(input, CMD_HISTORY) == 0) {
            if (check_session_valid(session, "history command")) {
                session->service->get_history(session->service, session->user);
            }
        }

        // Handle unknown command
        else if (input[0] == '/') {
            log_message(ERROR, "Unknown command. Type /help for available commands");
        }

        // Handle text without command prefix
        else if (strlen(input) > 0) {
            if (session->user != NULL && session->isLogin) {
                log_message(INFO, "Direct message sending requires using /u-msg <user_id> <message> or /msg <group_id> <message>");
            } else {
                log_message(ERROR, "You must be logged in to send messages");
            }
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    // Initialize session
    Session *session = createSession();
    if (session == NULL) {
        log_message(ERROR, "Failed to create session");
        return 1;
    }

    // Ignore SIGPIPE signal
    signal(SIGPIPE, SIG_IGN);

    // Parse command line arguments for server address
    char *server_address = "127.0.0.1";
    int server_port = 1609;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--server") == 0 && i + 1 < argc) {
            server_address = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            server_port = atoi(argv[i + 1]);
            i++;
        }
    }

    printf("Connecting to server %s:%d...\n", server_address, server_port);
    session->connect(session, server_address, server_port);

    if (!session->connected) {
        log_message(ERROR, "Connection failed, exiting");
        destroySession(session);
        return 1;
    }

    log_message(INFO, "Connected to server successfully");

    // Create thread for keyboard input
    pthread_t keyboard_thread;
    if (pthread_create(&keyboard_thread, NULL, keyboard_input_handler, session) != 0) {
        log_message(ERROR, "Failed to create keyboard input thread");
        session->close(session);
        destroySession(session);
        return 1;
    }

    // Wait for keyboard thread to finish
    pthread_join(keyboard_thread, NULL);

    // Clean up and exit
    session->close(session);
    destroySession(session);

    log_message(INFO, "Application exiting");
    return 0;
}