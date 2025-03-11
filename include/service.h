#ifndef SERVICE_H
#define SERVICE_H

#include "session.h"
#include "message.h"

typedef struct Service Service;
typedef struct Session Session;

struct Service {
    Session* session;

    void (*login_success)(Service* self);
    void (*server_message)(Service* self, Message* message);
    void (*get_online_users)(Service* self);
};

Service* createService(Session* session);
void destroyService(Service* service);
void service_login_success(Service* service);
void server_message(Service* service, Message* message);
void service_get_online_users(Service* service);


#endif