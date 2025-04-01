// //
// // Created by vawnwuyest on 4/1/25.
// //
//
// #ifndef REQUEST_MANAGER_H
// #define REQUEST_MANAGER_H
//
// #include "session.h"
//
// typedef void (*ResponseCallback)(const char* response);
//
// typedef struct {
//     int request_id;
//     ResponseCallback callback;
// } RequestEntry;
//
// void send_request_with_callback(Session* session, const char* request, int request_id, ResponseCallback callback);
// void handle_server_response(Session* session, const char* response);
//
// void request_create_group(Session* session, const char* group_name);
// void on_create_group_response(const char* response);
//
// #endif // REQUEST_MANAGER_H
//
