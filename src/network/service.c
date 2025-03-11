#include "service.h"
#include "session.h"
#include "log.h"
#include <stdlib.h>
#include "cmd.h"
#include "message.h"

void service_login_success(Service* service);
void server_message(Service* service, Message* message);

Service* createService(Session* session) {
    if (session == NULL) {
        return NULL;
    }
    Service* service = (Service*)malloc(sizeof(Service));
    if (service == NULL) {
        return NULL;
    }
    
    service->session = session;
    // service->login_success = service_login_success;
    service->server_message = server_message;
    service->get_online_users = service_get_online_users;
    return service;
}

void destroyService(Service* service) {
    if (service != NULL) {
        free(service);
    }
}

void server_message(Service* service, Message* message) {
    if (service == NULL || message == NULL) {
        return;
    }

    char content[1024] = {0};
    message_read_string(message, content, sizeof(content));
    log_message(INFO, "Server message: %s", content);
}

void service_get_online_users(Service* service) {
    if (service == NULL) {
        return;
    }
    
    Message* message = message_create(GET_ONLINE_USERS);
    if (message == NULL) {
        log_message(ERROR, "Failed to create message");
        return;
    }
    //write anything for passing the encryption
    message_write_int(message, 0);
    if(!service->session) {
        log_message(ERROR, "Session is NULL");
        return;
    }
    session_send_message(service->session, message);
}