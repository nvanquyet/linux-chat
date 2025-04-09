#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "session.h"
#include "log.h"
#include <signal.h>
#include <ui_controller.h>
#include <gtk/gtk.h>

int main(int argc, char *argv[]) {
    // Initialize session
    gtk_init(&argc, &argv);
     main_session = createSession();
    if (main_session == NULL) {
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
    main_session->connect(main_session, server_address, server_port);

    if (!main_session->connected) {
        log_message(ERROR, "Connection failed, exiting");
        destroySession(main_session);
        return 1;
    }

    log_message(INFO, "Connected to server successfully");
    init(main_session);
    on_show_ui(LOGIN);
    gtk_main();
    // Clean up and exit
    main_session->close(main_session);
    destroySession(main_session);

    log_message(INFO, "Application exiting");
    return 0;
}