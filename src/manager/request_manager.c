// //
// // Created by vawnwuyest on 4/1/25.
// //
// #include "../include/request_manager.h"
// #include <stdio.h>
// #include <stdlib.h>
//
// #define MAX_PENDING_REQUESTS 100
//
// static RequestEntry pending_requests[MAX_PENDING_REQUESTS];
//
// void send_request_with_callback(Session* session, const char* request, int request_id, ResponseCallback callback) {
//     session->send(session, request);
//
//     for (int i = 0; i < MAX_PENDING_REQUESTS; i++) {
//         if (pending_requests[i].request_id == 0) {
//             pending_requests[i].request_id = request_id;
//             pending_requests[i].callback = callback;
//             break;
//         }
//     }
// }
//
// void handle_server_response(Session* session, const char* response) {
//     int request_id = extract_request_id(response);
//
//     for (int i = 0; i < MAX_PENDING_REQUESTS; i++) {
//         if (pending_requests[i].request_id == request_id) {
//             if (pending_requests[i].callback) {
//                 pending_requests[i].callback(response);
//             }
//             pending_requests[i].request_id = 0;
//             break;
//         }
//     }
// }
//
// void on_create_group_response(const char* response) {
//     printf("Group creation response: %s\n", response);
// }
//
// void request_create_group(Session* session, const char* group_name) {
//     int request_id = rand();
//     char request[256];
//     sprintf(request, "{ \"type\": \"create_group\", \"request_id\": %d, \"group_name\": \"%s\" }", request_id, group_name);
//     send_request_with_callback(session, request, request_id, on_create_group_response);
// }
