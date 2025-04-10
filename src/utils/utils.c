#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "m_utils.h"
#include "log.h"
#include <openssl/sha.h>
#include <stdint.h>
#include "regex.h"
#include <stdbool.h>
#include <glib.h>
#include <glib/gstrfuncs.h>

bool is_port_available(int port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        return false;
    }

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        close(sock);
        return false;
    }

    close(sock);
    return true;
}

int utils_next_int(int max)
{
    return rand() % max;
}

int utils_mod_exp(int base, int exp, int mod)
{
    int result = 1;
    base = base % mod;

    while (exp > 0)
    {
        if (exp % 2 == 1)
        {
            result = (result * base) % mod;
        }

        exp = exp >> 1;
        base = (base * base) % mod;
    }

    return result;
}


void generate_aes_key_from_K(uint32_t K, unsigned char *aes_key) {
    if (aes_key == NULL) {
        log_message(ERROR, "AES key buffer is NULL");
        return;
    }
    
    unsigned char hash[SHA256_DIGEST_LENGTH]; 
    SHA256((unsigned char *)&K, sizeof(K), hash);
    
    memcpy(aes_key, hash, 32);
}

bool validate_username_password(char *username, char *password) {
    if (username == NULL || password == NULL) {
        return false;
    }
    
    // Username validation with regex (this part works fine)
    regex_t username_regex;
    int username_result;
    char error_buffer[100];
    
    // Username: 3-20 chars, starts with letter, allows letters/numbers/underscore
    if (regcomp(&username_regex, "^[a-zA-Z][a-zA-Z0-9_]{2,19}$", REG_EXTENDED) != 0) {
        log_message(ERROR, "Failed to compile username regex");
        return false;
    }
    
    username_result = regexec(&username_regex, username, 0, NULL, 0);
    regfree(&username_regex);  // Free username regex right after use
    
    if (username_result != 0) {
        regerror(username_result, &username_regex, error_buffer, sizeof(error_buffer));
        log_message(ERROR, "Username validation failed: Must be 3-20 characters, start with a letter, and contain only letters, numbers, and underscores");
        return false;
    }
    
    // Password validation without regex lookaheads
    size_t password_len = strlen(password);
    
    // Check password length
    if (password_len < 6 || password_len > 64) {
        log_message(ERROR, "Password validation failed: Password must be between 6-64 characters");
        return false;
    }
    
    // Check for required character types
    bool has_digit = false;
    bool has_lowercase = false;
    bool has_uppercase = false;
    bool has_invalid_char = false;
    
    for (size_t i = 0; i < password_len; i++) {
        char c = password[i];
        
        if (c >= '0' && c <= '9') {
            has_digit = true;
        } else if (c >= 'a' && c <= 'z') {
            has_lowercase = true;
        } else if (c >= 'A' && c <= 'Z') {
            has_uppercase = true;
        } else if (c == ' ') {
            // Specifically reject spaces
            log_message(ERROR, "Password validation failed: Password cannot contain spaces");
            return false;
        } else {
            // Check if it's a valid special character
            if (strchr("!@#$%^&*()_+-=[]{};:'\",.<>/?\\|", c) == NULL) {
                has_invalid_char = true;
                break;
            }
        }
    }
    
    if (!has_digit) {
        log_message(ERROR, "Password validation failed: Password must contain at least one digit");
        return false;
    }
    
    if (!has_lowercase) {
        log_message(ERROR, "Password validation failed: Password must contain at least one lowercase letter");
        return false;
    }
    
    if (!has_uppercase) {
        log_message(ERROR, "Password validation failed: Password must contain at least one uppercase letter");
        return false;
    }
    
    if (has_invalid_char) {
        log_message(ERROR, "Password validation failed: Password contains invalid characters");
        return false;
    }
    
    return true;
}

char *hash_password(const char *password) {
    if (password == NULL) return NULL;

    GChecksum *checksum = g_checksum_new(G_CHECKSUM_SHA256);
    g_checksum_update(checksum, (const guchar *)password, strlen(password));

    gchar *hashed = g_strdup(g_checksum_get_string(checksum));
    g_checksum_free(checksum);

    return hashed; // Đừng quên g_free sau khi dùng xong!
}