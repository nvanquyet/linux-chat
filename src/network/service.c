#include "service.h"

#include <m_utils.h>

#include "session.h"
#include "log.h"
#include <stdlib.h>
#include "cmd.h"
#include "json_utils.h"
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
    
    service->server_message = server_message;
    service->get_online_users = service_get_online_users;

    service->get_group_list = service_get_group_list;
    service->create_group = service_create_group;
    service->join_group = service_join_group;
    service->leave_group = service_leave_group;
    service->delete_group = service_delete_group;

    service->get_history = service_get_history;
    service->get_history_message = service_get_history_message;
    service->search_users = service_search_users;
    service->service_logout = service_logout;

    service->send_group_message = service_send_group_message;
    service->send_user_message = service_send_user_message;
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
    
    Message* message = message_create(GET_USERS);
    if (message == NULL) {
        log_message(ERROR, "Failed to create message");
        return;
    }
    
    message_write_int(message, 0);
    if(!service->session) {
        log_message(ERROR, "Session is NULL");
        return;
    }
    session_send_message(service->session, message);
}

void service_search_users(Service* service, const char* text)
{
    if (service == NULL || text == NULL)
    {
        return;
    }
    if(!service->session)
    {
        log_message(ERROR, "Session is NULL");
        return;
    }
    Message *message = message_create(SEARCH_USERS);
    if (message == NULL)
        return;
    message_write_int(message, service->session->user->id);
    message_write_string(message, text);
    session_send_message(service->session, message);

    log_message(INFO, "Search users %s", text);
}


void service_logout(User* self)
{
    self->logout(self);
}


void service_get_group_list(Service* service,  User* user) {
    if (service == NULL) return;

    Message* message = message_create(GET_JOINED_GROUPS);
    if (message == NULL) {
        log_message(ERROR, "Failed to create message");
        return;
    }
    
    message_write_int(message, user->id);
    if(!service->session) {
        log_message(ERROR, "Session is NULL");
        return;
    }
    session_send_message(service->session, message);
}

void service_create_group(Service* service, User* user, const char* group_name, const char *group_password) {
    if (service == NULL || group_name == NULL) {
        return;
    }
    Message* message = message_create(CREATE_GROUP);

    if (message == NULL) {
        log_message(ERROR, "Failed to create message");
        return;
    }

    if(!service->session) {
        log_message(ERROR, "Session is NULL");
        return;
    }
    message_write_int(message, user->id);
    message_write_string(message, group_name);
    message_write_string(message, hash_password(group_password));
    
    session_send_message(service->session, message);
}

void service_join_group(Service* service, User* user, const char* group_name, const char * group_password) {
    if (service == NULL) {
        log_message(ERROR, "Service is NULL");
        return;
    }

    if (user == NULL) {
        log_message(ERROR, "User is NULL");
        return;
    }

    
    Message* message = message_create(JOIN_GROUP);
    if (message == NULL) {
        log_message(ERROR, "Failed to create message");
        return;
    }

    
    message_write_int(message, user->id);
    message_write_string(message, group_name);
    message_write_string(message, hash_password(group_password));
    message_write_string(message, user->username);

    
    if (service->session == NULL) {
        log_message(ERROR, "Session is NULL");
        free(message);  
        return;
    }

    
    session_send_message(service->session, message);
}

void service_leave_group(Service* service, User* user, int group_id) {
    if (service == NULL) {
        log_message(ERROR, "Service is NULL");
        return;
    }

    if (user == NULL) {
        log_message(ERROR, "User is NULL");
        return;
    }

    Message* message = message_create(LEAVE_GROUP);
    if (message == NULL) {
        log_message(ERROR, "Failed to create message");
        return;
    }

    
    message_write_int(message, group_id < 0 ? -group_id : group_id);
    message_write_int(message, user->id);

    if (!service->session) {
        log_message(ERROR, "Session is NULL");
        free(message);  
        return;
    }

    log_message(INFO, "Leaving group %d", group_id < 0 ? -group_id : group_id);
    session_send_message(service->session, message);
}

void service_delete_group(Service* service, User* user, int group_id) {
    if (service == NULL) return;
    if (user == NULL) {
        log_message(ERROR, "User is NULL");
        return;
    }

    Message* message = message_create(DELETE_GROUP);
    if (message == NULL) {
        log_message(ERROR, "Failed to create message");
        return;
    }
    
    message_write_int(message, group_id > 0 ? group_id : -group_id);
    message_write_int(message, user->id);
    if(!service->session) {
        log_message(ERROR, "Session is NULL");
        free(message);
        return;
    }

    session_send_message(service->session, message);
}


void service_send_group_message(Service* service,  User* user, int group_id, const char* message) {
    if (service == NULL) return;

    Message* m = message_create(GROUP_MESSAGE);
    if (m == NULL) {
        log_message(ERROR, "Failed to create message");
        return;
    }
    
    message_write_int(m, user->id);
    message_write_int(m, group_id);
    message_write_string(m, user->username);
    message_write_string(m, message);
    if(!service->session) {
        log_message(ERROR, "Session is NULL");
        return;
    }
    session_send_message(service->session, m);
    log_message(INFO, "Sending message %s", message);
}


void service_send_user_message(Service* service,  User* user, int user_id, const char* message) {
    if (service == NULL) return;

    Message* m = message_create(USER_MESSAGE);
    if (m == NULL) {
        log_message(ERROR, "Failed to create message");
        return;
    }
    
    message_write_int(m, user->id);
    message_write_int(m, user_id);
    message_write_string(m, user->username);
    message_write_string(m, message);
    if(!service->session) {
        log_message(ERROR, "Session is NULL");
        return;
    }
    session_send_message(service->session, m);
    log_message(INFO, "Sending message %s", message);
}

void service_get_history(Service* service, User* user) {
    if (service == NULL) return;
    if (user == NULL) {
        log_message(ERROR, "User is NULL");
        return;
    }
    Message* message = message_create(GET_CHAT_HISTORY);
    if (message == NULL) {
        log_message(ERROR, "Failed to create message");
        return;
    }
    message_write_int(message, user->id);
    session_send_message(service->session, message);
}

void service_get_history_message(Service* service,  User* user, int target_id)
{
    if (service == NULL) return;
    Message* m = target_id > 0 ? message_create(GET_USERS_MESSAGE) : message_create(GET_GROUPS_MESSAGE);
    if (m == NULL)
    {
        log_message(ERROR, "Failed to create message");
        return;
    }
    log_message(INFO, "Get History between %d and %d", user->id, target_id > 0 ? target_id : -target_id);

    message_write_int(m, user->id);
    message_write_int(m, target_id > 0 ? target_id : -target_id);
    session_send_message(service->session, m);
}