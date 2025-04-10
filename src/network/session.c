
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <openssl/rand.h>

#include "aes_utils.h"
#include "cmd.h"
#include "controller.h"
#include "log.h"
#include "m_utils.h"
#include "message.h"
#include "service.h"
#include "session.h"
#include "user.h"

typedef struct
{
    Message **messages;
    int capacity;
    int size;
    pthread_mutex_t mutex;
} MessageQueue;

typedef struct
{
    Session *session;
    pthread_t thread;
    bool running;
} MessageCollector;

typedef struct
{
    Session *session;
    MessageQueue *queue;
    pthread_t thread;
    bool running;
} Sender;

typedef struct
{
    byte *key;
    MessageCollector *collector;
    Sender *sender;
    bool sendKeyComplete;
    bool tradingKey;
    bool isClosed;
} SessionPrivate;

typedef struct
{
    uint8_t p;
    uint8_t g;
    uint16_t bb;
    uint16_t A;
    uint16_t B;
    uint16_t K;
} Key;

void *sender_thread(void *arg);
void *collector_thread(void *arg);
Message *read_message(Session *session);
void process_message(Session *session, Message *msg);
bool do_send_message(Session *session, Message *msg);
void trade_key(Session *session, Message *msg);
void send_private_key(Session *session, Message *msg);
void compute_shared_key(Session *session, Message *msg);
void clean_network(Session *session);
void session_close_message(Session *session);

void session_do_connect(Session *session, char *ip, int port);
void session_connect(Session *self, char *ip, int port);
void session_init_network(Session *session);
void *session_init_network_ptr(void *arg);

void session_client_ok(Session *self);
bool session_is_connected(Session *self);
void session_disconnect(Session *self);
void session_on_message(Session *self, Message *msg);
void session_process_message(Session *self, Message *msg);
bool session_do_send_message(Session *self, Message *msg);
void session_close(Session *self);
Message *session_read_message(Session *self);

MessageQueue *message_queue_create(int initial_capacity);
void message_queue_add(MessageQueue *queue, Message *message);
Message *message_queue_get(MessageQueue *queue, int index);
Message *message_queue_remove(MessageQueue *queue, int index);
void message_queue_destroy(MessageQueue *queue);

Session *createSession()
{
    Session *session = (Session *)malloc(sizeof(Session));
    if (session == NULL)
    {
        return NULL;
    }

    SessionPrivate *private = (SessionPrivate *)malloc(sizeof(SessionPrivate));
    if (private == NULL)
    {
        free(session);
        return NULL;
    }

    Key *key = (Key *)malloc(sizeof(Key));
    if (key == NULL)
    {
        free(session);
        return NULL;
    }
    session->connected = false;
    session->connecting = true;
    session->clientOK = false;

    session->isConnected = session_is_connected;
    session->setHandler = session_set_handler;
    session->setService = session_set_service;
    session->sendMessage = session_send_message;
    session->close = session_close;
    session->clientOk = session_client_ok;
    session->doSendMessage = session_do_send_message;
    session->disconnect = session_disconnect;
    session->onMessage = session_on_message;
    session->processMessage = session_process_message;
    session->readMessage = session_read_message;
    session->closeMessage = session_close_message;
    session->socket = -1;
    session->isLogin = false;

    session->doConnect = session_do_connect;
    session->connect = session_connect;
    session->initNetwork = session_init_network;

    private->key = NULL;
    private->sendKeyComplete = false;
    private->isClosed = false;
    private->tradingKey = false;
    

    private->sender = (Sender *)malloc(sizeof(Sender));
    private->sender->session = session;
    private->sender->queue = message_queue_create(10);
    private->sender->running = false;

    private->collector = (MessageCollector *)malloc(sizeof(MessageCollector));
    private->collector->session = session;
    private->collector->running = false;

    key->A = 0;
    key->B = 0;
    key->K = 0;
    key->g = 0;
    key->p = 0;
    key->bb = 0;
    session->_key = key;
    session->_private = private;

    session->handler = createController(session);
    session->service = createService(session);
    session->handler->service = session->service;
    session->user = NULL;
    return session;
}

void destroySession(Session *session)
{
    if (session != NULL)
    {
        SessionPrivate *private = (SessionPrivate *)session->_private;

        if (private != NULL)
        {
            if (private->sender != NULL)
            {
                private->sender->running = false;
                pthread_join(private->sender->thread, NULL);

                if (private->sender->queue != NULL)
                {
                    message_queue_destroy(private->sender->queue);
                }
                free(private->sender);
            }

            if (private->collector != NULL)
            {
                private->collector->running = false;
                pthread_join(private->collector->thread, NULL);
                free(private->collector);
            }

            if (private->key != NULL)
            {
                free(private->key);
            }

            free(private);
        }

        if (session->socket != -1)
        {
            close(session->socket);
        }

        free(session);
    }
}

bool session_is_connected(Session *self)
{
    return self->connected;
}

void session_set_handler(Session *session, Controller *handler)
{
    if (session != NULL)
    {
        session->handler = handler;
    }
}

void session_set_service(Session *session, Service *service)
{
    if (session != NULL)
    {
        session->service = service;
    }
}

void session_connect(Session *session, char *ip, int port)
{

    SessionPrivate *private = (SessionPrivate *)session->_private;
    private->isClosed = false;
    private->sendKeyComplete = false;
    session->port = port;
    session->ip = ip;

    session_init_network(session);
}

void *session_init_network_ptr(void *arg)
{
    log_message(INFO, "Init network");
    Session *session = (Session *)arg;
    session_init_network(session);
    return NULL;
}

void session_init_network(Session *session)
{
    session_do_connect(session, session->ip, session->port);
}

void session_do_connect(Session *session, char *ip, int port)
{
    SessionPrivate *private = (SessionPrivate *)session->_private;
    if (private->isClosed)
    {
        return;
    }

    session->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (session->socket < 0)
    {
        log_message(ERROR, "Failed to create socket");
        return;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(session->port);
    server_addr.sin_addr.s_addr = inet_addr(session->ip);

    if (connect(session->socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        log_message(ERROR, "Failed to connect to server");
        return;
    }
    log_message(INFO, "Connected to server");
    private->sendKeyComplete = false;
    private->isClosed = false;

    session->connecting = false;
    session->connected = true;
    session->isRunning = true;

    private->sender->running = true;
    pthread_create(&private->sender->thread, NULL, sender_thread, private->sender);

    private->collector->running = true;
    pthread_create(&private->collector->thread, NULL, collector_thread, private->collector);

    Message *msg = message_create(GET_SESSION_ID);
    if (msg == NULL)
    {
        log_message(ERROR, "Failed to create message");
        return;
    }
    session->doSendMessage(session, msg);
    message_destroy(msg);
}

void session_send_message(Session *session, Message *message)
{
    if (session == NULL || message == NULL)
    {
        return;
    }


    SessionPrivate *private = (SessionPrivate *)session->_private;

    if (session->connected && private->sender != NULL && private->sender->running)
    {
        message_queue_add(private->sender->queue, message);
    }
}

bool session_do_send_message(Session *session, Message *msg)
{
    if (session == NULL || msg == NULL)
    {
        return false;
    }
    if (!do_send_message(session, msg))
    {
        log_message(ERROR, "Failed to send message");
        return false;
    }
    return true;
}

void session_close_message(Session *self)
{
    if (self == NULL)
    {
        return;
    }

    SessionPrivate *private = (SessionPrivate *)self->_private;
    if (!private || private->isClosed)
    {
        return;
    }

    private->isClosed = true;

    if (self->handler != NULL)
    {
        self->handler->onDisconnected(self->handler);
    }
    else
    {
        log_message(ERROR, "Failed to call onDisconnected");
    }
    session_close(self);
}
void session_close(Session *self)
{
    if (self == NULL)
    {
        return;
    }

    clean_network(self);
}

void session_disconnect(Session *self)
{
    if (self == NULL || !self->connected)
    {
        return;
    }

    close(self->socket);
    self->connected = false;
}

void trade_key(Session *session, Message *msg)
{
    if (session == NULL)
    {
        return;
    }

    SessionPrivate *private = (SessionPrivate *)session->_private;
    if (private->sendKeyComplete)
    {
        return;
    }

    if (private->tradingKey)
    {
    }

    switch (msg->command)
    {
    case TRADE_DH_PARAMS:
        send_private_key(session, msg);
        break;
    case TRADE_KEY:
        compute_shared_key(session, msg);
        break;
    default:
        break;
    }
}

void send_private_key(Session *session, Message *msg)
{
    Key *key = (Key *)session->_key;
    SessionPrivate *private = (SessionPrivate *)session->_private;
    key->p = msg->buffer[0];
    key->g = msg->buffer[1];
    if (key->p == 0 || key->g == 0)
    {
        log_message(ERROR, "Invalid DH params, p: %d, g: %d", key->p, key->g);
        return;
    }

    key->bb = utils_next_int(65535);
    key->B = utils_mod_exp(key->g, key->bb, key->p);
    if (key->B == 0)
    {
        log_message(ERROR, "Invalid public key");
        return;
    }

    Message *message = message_create(TRADE_DH_PARAMS);
    message_write(message, &key->B, sizeof(key->B));
    session->doSendMessage(session, message);
    message_destroy(message);
    private->sendKeyComplete = false;
}

void compute_shared_key(Session *session, Message *msg)
{
    SessionPrivate *private = (SessionPrivate *)session->_private;
    Key *key = (Key *)session->_key;
    key->A = msg->buffer[0];
    if (key->A == 0)
    {
        log_message(ERROR, "Invalid public key A: %d", key->A);
        return;
    }

    key->K = utils_mod_exp(key->A, key->bb, key->p);
    if (key->K == 0)
    {
        log_message(ERROR, "Invalid shared key");
        return;
    }
    private->key = (unsigned char *)malloc(32);
    generate_aes_key_from_K(key->K, private->key);
    private->sendKeyComplete = true;
}

void session_client_ok(Session *self)
{
    if (self == NULL || self->clientOK)
    {
        return;
    }

    if (self->user == NULL)
    {
        log_message(ERROR, "Client %d: User not logged in");
        session_disconnect(self);
        return;
    }

    bool load_success = true;

    if (load_success)
    {
        self->clientOK = true;

        log_message(INFO, "Client %d: logged in successfully");
    }
    else
    {
        log_message(ERROR, "Client %d: Failed to load player data");
        session_disconnect(self);
    }
}

void session_on_message(Session *self, Message *msg)
{
    if (self == NULL || msg == NULL)
    {
        return;
    }

    SessionPrivate *private = (SessionPrivate *)self->_private;
    if (private->isClosed)
    {
        return;
    }

    if (self->handler != NULL)
    {
        (self->handler, msg);
    }
}

void clean_network(Session *session)
{
    if (session == NULL)
    {
        return;
    }
    /*
        SessionPrivate *private = (SessionPrivate *)session->_private;

        if (session->user != NULL && !session->user->isCleaned)
        {
        }
     */
    session->connected = false;

    if (session->socket != -1)
    {
        close(session->socket);
        session->socket = -1;
    }

    session->handler = NULL;
    session->service = NULL;
    session->isRunning = false;
}

void *sender_thread(void *arg)
{
    Sender *sender = (Sender *)arg;
    Session *session = sender->session;
    MessageQueue *queue = sender->queue;

    while (session->connected && sender->running)
    {
        SessionPrivate *private = (SessionPrivate *)session->_private;

        if (private->sendKeyComplete)
        {
            while (queue->size > 0)
            {
                
                Message *msg = message_queue_remove(queue, 0);
                if (msg != NULL)
                {
                    do_send_message(session, msg);
                    message_destroy(msg);
                }

            }
        }

        usleep(10000);
    }

    return NULL;
}

void *collector_thread(void *arg)
{
    MessageCollector *collector = (MessageCollector *)arg;
    Session *session = collector->session;
    SessionPrivate *private = (SessionPrivate *)session->_private;

    while (session->connected && collector->running)
    {
        Message *message = session_read_message(session);
        if (message != NULL)
        {
            if (!private->sendKeyComplete)
            {
                trade_key(session, message);
            }
            else
            {
                process_message(session, message);
            }
        }
        else
        {
            break;
        }
    }

    if (!session->connecting)
    {
        session_close_message(session);
    }

    return NULL;
}

Message *session_read_message(Session *session)
{
    if (session == NULL)
    {
        return NULL;
    }

    SessionPrivate *private = (SessionPrivate *)session->_private;

    uint8_t command;
    if (recv(session->socket, &command, sizeof(command), 0) <= 0)
    {
        return NULL;
    }

    if (command == GET_SESSION_ID || command == TRADE_KEY || command == TRADE_DH_PARAMS)
    {

        uint32_t size_network;
        if (recv(session->socket, &size_network, sizeof(size_network), 0) <= 0)
        {
            log_message(ERROR, "Failed to receive unencrypted message size");
            return NULL;
        }
        uint32_t size = ntohl(size_network);

        Message *msg = message_create(command);
        if (msg == NULL)
        {
            log_message(ERROR, "Failed to create message");
            return NULL;
        }

        if (size > 0)
        {

            free(msg->buffer);
            msg->buffer = (unsigned char *)malloc(size);
            if (msg->buffer == NULL)
            {
                log_message(ERROR, "Failed to allocate buffer for unencrypted message");
                message_destroy(msg);
                return NULL;
            }
            msg->size = size;

            size_t total_read = 0;
            while (total_read < size)
            {
                ssize_t bytes_read = recv(session->socket, msg->buffer + total_read,
                                          size - total_read, 0);
                if (bytes_read <= 0)
                {
                    log_message(ERROR, "Failed to receive unencrypted data");
                    message_destroy(msg);
                    return NULL;
                }
                total_read += bytes_read;
            }
            msg->position = size;
        }

        return msg;
    }

    unsigned char iv[16];  
    int bytes_received = recv(session->socket, iv, sizeof(iv), 0);
    if (bytes_received <= 0) {
        log_message(ERROR, "Failed to receive IV, bytes received: %d", bytes_received);
        return NULL;
    }


    uint32_t original_size;
    if (recv(session->socket, &original_size, sizeof(original_size), 0) <= 0)
    {
        log_message(ERROR, "Failed to receive original size");
        return NULL;
    }
    original_size = ntohl(original_size);

    uint32_t encrypted_size;
    if (recv(session->socket, &encrypted_size, sizeof(encrypted_size), 0) <= 0)
    {
        log_message(ERROR, "Failed to receive encrypted size");
        return NULL;
    }
    encrypted_size = ntohl(encrypted_size);

    Message *msg = message_create(command);
    if (msg == NULL)
    {
        log_message(ERROR, "Failed to create message");
        return NULL;
    }

    free(msg->buffer);
    msg->buffer = (unsigned char *)malloc(encrypted_size);
    if (msg->buffer == NULL)
    {
        log_message(ERROR, "Failed to allocate buffer");
        message_destroy(msg);
        return NULL;
    }
    msg->size = encrypted_size;

    size_t total_read = 0;
    while (total_read < encrypted_size)
    {
        ssize_t bytes_read = recv(session->socket, msg->buffer + total_read,
                                  encrypted_size - total_read, 0);
        if (bytes_read <= 0)
        {
            log_message(ERROR, "Failed to receive encrypted data");
            message_destroy(msg);
            return NULL;
        }
        total_read += bytes_read;
    }
    msg->position = encrypted_size;

    if (private->key == NULL)
    {
        private->key = (unsigned char *)malloc(32);
        if (private->key == NULL)
        {
            log_message(ERROR, "Failed to allocate key");
            message_destroy(msg);
            return NULL;
        }
        memcpy(private->key, "test_secret_key_for_aes_256_cipher", 32);
    }

    if (!message_decrypt(msg, private->key, iv))
    {
        log_message(ERROR, "Failed to decrypt message");
        message_destroy(msg);
        return NULL;
    }

    msg->position = 0;
    return msg;
}

void process_message(Session *session, Message *msg)
{
    if (session == NULL || msg == NULL)
    {
        return;
    }

    SessionPrivate *private = (SessionPrivate *)session->_private;
    if (!private->isClosed)
    {
        Controller *handler = session->handler;
        if (handler != NULL)
        {
            handler->onMessage(handler, msg);
        }
        else
        {
            log_message(ERROR, "Failed to call onMessage");
        }
    }
}

bool do_send_message(Session *session, Message *msg)
{   
    if (session == NULL || msg == NULL)
    {
        return false;
    }

    if (msg->command == GET_SESSION_ID || msg->command == TRADE_KEY || msg->command == TRADE_DH_PARAMS)
    {
        if (send(session->socket, &msg->command, sizeof(uint8_t), 0) < 0)
        {
            log_message(ERROR, "Failed to send unencrypted message command");
            return false;
        }

        uint32_t net_size = htonl((uint32_t)msg->position);
        if (send(session->socket, &net_size, sizeof(net_size), 0) < 0)
        {
            log_message(ERROR, "Failed to send unencrypted message size");
            return false;
        }

        if (msg->position > 0)
        {
            if (send(session->socket, msg->buffer, msg->position, 0) < 0)
            {
                log_message(ERROR, "Failed to send unencrypted message data");
                return false;
            }
        }
        return true;
    }

    SessionPrivate *private = (SessionPrivate *)session->_private;

    unsigned char iv[16];
    if (RAND_bytes(iv, sizeof(iv)) != 1)
    {
        log_message(ERROR, "Failed to generate random IV");
        return false;
    }

    size_t original_size = msg->position;

    if (!message_encrypt(msg, private->key, iv))
    {
        log_message(ERROR, "Failed to encrypt message");
        return false;
    }

    if (send(session->socket, &msg->command, sizeof(uint8_t), 0) < 0)
    {
        log_message(ERROR, "Failed to send message command");
        return false;
    }

    if (send(session->socket, iv, sizeof(iv), 0) < 0)
    {
        log_message(ERROR, "Failed to send message IV");
        return false;
    }

    uint32_t net_original_size = htonl((uint32_t)original_size);
    if (send(session->socket, &net_original_size, sizeof(net_original_size), 0) < 0)
    {
        log_message(ERROR, "Failed to send original size");
        return false;
    }

    uint32_t net_encrypted_size = htonl((uint32_t)msg->position);
    if (send(session->socket, &net_encrypted_size, sizeof(net_encrypted_size), 0) < 0)
    {
        log_message(ERROR, "Failed to send encrypted size");
        return false;
    }

    if (send(session->socket, msg->buffer, msg->position, 0) < 0)
    {
        log_message(ERROR, "Failed to send encrypted data");
        return false;
    }

    return true;
}
MessageQueue *message_queue_create(int initial_capacity)
{
    MessageQueue *queue = (MessageQueue *)malloc(sizeof(MessageQueue));
    if (queue == NULL)
    {
        return NULL;
    }

    queue->messages = (Message **)malloc(sizeof(Message *) * initial_capacity);
    queue->capacity = initial_capacity;
    queue->size = 0;
    pthread_mutex_init(&queue->mutex, NULL);

    return queue;
}

void message_queue_add(MessageQueue *queue, Message *message)
{
    if (queue == NULL || message == NULL)
    {
        return;
    }
    pthread_mutex_lock(&queue->mutex);

    if (queue->size >= queue->capacity)
    {
        int new_capacity = queue->capacity * 2;
        Message **new_messages = (Message **)realloc(queue->messages, sizeof(Message *) * new_capacity);
        if (new_messages == NULL)
        {
            pthread_mutex_unlock(&queue->mutex);
            return;
        }
        queue->messages = new_messages;
        queue->capacity = new_capacity;
    }

    queue->messages[queue->size++] = message;

    pthread_mutex_unlock(&queue->mutex);

}

Message *message_queue_get(MessageQueue *queue, int index)
{
    if (queue == NULL || index < 0 || index >= queue->size)
    {
        return NULL;
    }

    pthread_mutex_lock(&queue->mutex);
    Message *msg = queue->messages[index];
    pthread_mutex_unlock(&queue->mutex);
    return msg;
}

Message *message_queue_remove(MessageQueue *queue, int index)
{
    if (queue == NULL || index < 0 || index >= queue->size)
    {
        return NULL;
    }

    pthread_mutex_lock(&queue->mutex);

    Message *msg = queue->messages[index];

    for (int i = index; i < queue->size - 1; i++)
    {
        queue->messages[i] = queue->messages[i + 1];
    }

    queue->size--;

    pthread_mutex_unlock(&queue->mutex);

    return msg;
}

void message_queue_destroy(MessageQueue *queue)
{
    if (queue == NULL)
    {
        return;
    }

    pthread_mutex_lock(&queue->mutex);

    free(queue->messages);

    pthread_mutex_unlock(&queue->mutex);
    pthread_mutex_destroy(&queue->mutex);

    free(queue);
}

void session_process_message(Session *self, Message *msg)
{
    if (self == NULL || msg == NULL)
    {
        return;
    }

    SessionPrivate *private = (SessionPrivate *)self->_private;
    if (private->isClosed)
    {
        return;
    }
    self->onMessage(self, msg);
}