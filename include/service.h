#ifndef SERVICE_H
#define SERVICE_H

#include "user.h"
#include "message.h"

typedef struct Service Service;
typedef struct Session Session;
typedef struct User User;
struct Service {
    Session* session;

    void (*login_success)(Service* self);
    void (*server_message)(Service* self, Message* message);
    void (*get_online_users)(Service* self);

    // Group Management
    void (*get_group_list)(Service* self,  User* user);
    void (*create_group)(Service* self, User* user, const char* group_name, const char * group_password);
    void (*join_group)(Service* self, User* user, const char* group_name, const char * group_password);
    void (*leave_group)(Service* self, User* user, int group_id);
    void (*delete_group)(Service* self, User* user, int group_id);
    void (*get_history)(Service* self, User* user);

    // Messaging
    void (*send_group_message)(Service* self, User* user, int group_id, const char* message);
    void (*send_user_message)(Service* self, User* user, int user_id, const char* message);
};

// Constructor & Destructor
Service* createService(Session* session);
void destroyService(Service* service);

//Event Handling
void server_message(Service* service, Message* message);
void service_get_online_users(Service* service);

// Group Handling
void service_get_group_list(Service* service,  User* user);
void service_create_group(Service* service, User* user, const char* group_name, const char * group_password);
void service_join_group(Service* service, User* user, const char* group_name, const char * group_password);
void service_leave_group(Service* service, User* user,int group_id);
void service_delete_group(Service* service,  User* user, int group_id);

// Messaging
void service_send_group_message(Service* service,  User* user, int group_id, const char* message);
void service_send_user_message(Service* service, User* user, int user_id, const char* message);

void service_get_history(Service* service,  User* user);
#endif
