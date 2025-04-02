#include "service.h"
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
    // service->login_success = service_login_success;
    service->server_message = server_message;
    service->get_online_users = service_get_online_users;

    service->get_group_list = service_get_group_list;
    service->create_group = service_create_group;
    service->join_group = service_join_group;
    service->leave_group = service_leave_group;
    service->delete_group = service_delete_group;

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

// Group Handling
void service_get_group_list(Service* service,  User* user) {
    if (service == NULL) return;

    Message* message = message_create(GET_JOINED_GROUPS);
    if (message == NULL) {
        log_message(ERROR, "Failed to create message");
        return;
    }
    //write anything for passing the encryption
    message_write_int(message, user->id);
    if(!service->session) {
        log_message(ERROR, "Session is NULL");
        return;
    }
    session_send_message(service->session, message);
    log_message(INFO, "Request get joined groups");
}

void service_create_group(Service* service, User* user, const char* group_name) {
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
    message_write_string(message, group_name);
    message_write_int(message, user->id);
    //Send to server
    session_send_message(service->session, message);
}
void service_join_group(Service* service, User* user, int group_id) {
    if (service == NULL) {
        log_message(ERROR, "Service is NULL");
        return;
    }

    if (user == NULL) {
        log_message(ERROR, "User is NULL");
        return;
    }

    // Tạo message để gửi tới server
    Message* message = message_create(JOIN_GROUP);
    if (message == NULL) {
        log_message(ERROR, "Failed to create message");
        return;
    }

    // Ghi dữ liệu vào message
    message_write_int(message, group_id);
    message_write_int(message, user->id);

    // Kiểm tra nếu session là NULL
    if (service->session == NULL) {
        log_message(ERROR, "Session is NULL");
        free(message);  // Giải phóng bộ nhớ message để tránh rò rỉ
        return;
    }

    // Gửi message đến server
    session_send_message(service->session, message);

    // Log thông tin tham gia nhóm
    log_message(INFO, "User %d is joining group %d", user->id, group_id);

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

    // Write anything for passing the encryption
    message_write_int(message, group_id);
    message_write_int(message, user->id);

    if (!service->session) {
        log_message(ERROR, "Session is NULL");
        free(message);  // Don't forget to free the message to avoid memory leak
        return;
    }

    log_message(INFO, "Leaving group %d", group_id);
    session_send_message(service->session, message);
}

void service_delete_group(Service* service, User* user, int group_id) {
    if (service == NULL) return;

    Message* message = message_create(DELETE_GROUP);
    if (message == NULL) {
        log_message(ERROR, "Failed to create message");
        return;
    }
    //write anything for passing the encryption
    message_write_int(message, group_id);
    message_write_int(message, user->id);
    if(!service->session) {
        log_message(ERROR, "Session is NULL");
        return;
    }
    session_send_message(service->session, message);
    log_message(INFO, "Deleting group %s", group_id);
}

// Messaging
void service_send_group_message(Service* service,  User* user, int group_id, const char* message) {
    if (service == NULL) return;

    Message* m = message_create(SEND_GROUP_MESSAGE);
    if (m == NULL) {
        log_message(ERROR, "Failed to create message");
        return;
    }
    //write anything for passing the encryption
    message_write_int(m, user->id);
    message_write_int(m, group_id);
    message_write_string(m, message);
    if(!service->session) {
        log_message(ERROR, "Session is NULL");
        return;
    }
    session_send_message(service->session, m);
    log_message(INFO, "Sending message %s", message);
}


// Messaging
void service_send_user_message(Service* service,  User* user, int user_id, const char* message) {
    if (service == NULL) return;

    Message* m = message_create(SEND_MESSAGE);
    if (m == NULL) {
        log_message(ERROR, "Failed to create message");
        return;
    }
    //write anything for passing the encryption
    message_write_int(m, user->id);
    message_write_int(m, user_id);
    message_write_string(m, message);
    if(!service->session) {
        log_message(ERROR, "Session is NULL");
        return;
    }
    session_send_message(service->session, m);
    log_message(INFO, "Sending message %s", message);
}
