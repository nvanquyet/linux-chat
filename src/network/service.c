#include "service.h"
#include "session.h"
#include "log.h"
#include <stdlib.h>

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
    service->login_success = service_login_success;
    service->server_message = server_message;
    return service;
}

void destroyService(Service* service) {
    if (service != NULL) {
        free(service);
    }
}



void service_login_success(Service* service) {
    if (service == NULL) {
        return;
    }
    
    log_message(INFO, "Logged in successfully");
    
}

void server_message(Service* service, Message* message) {
    if (service == NULL || message == NULL) {
        return;
    }

    char content[1024] = {0};
    message_read_string(message, content, sizeof(content));
    log_message(INFO, "Server message: %s", content);
}