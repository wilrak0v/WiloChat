#include <sodium/crypto_sign.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <sodium.h>
#include "btmp.h"
#include "db.h"
#include "mongoose.h"
#include "network.h"

#define NB_OF_MSG_TYPE 7

struct Session *sessions = NULL;

int hex_to_bytes(uint8_t *hex, uint8_t *bytes, size_t bytes_len)
{
    char *hex2 = (char *)hex;
    for (size_t i = 0; i < bytes_len; i++) {
        if (sscanf(hex2 + (i * 2), "%02hhx", &bytes[i]) != 1) {
            return -1;
        }
    }
    return 0;
}

void generate_challenge(uint8_t challenge[32])
{
    FILE *f = fopen("/dev/urandom", "r");
    if (f)
    {
        fread(challenge, 1, 32, f);
        fclose(f);
    }
    else
    {
        printf("ERROR: There is no '/dev/urandom'\n");
    }
}

int get_type(char *buf)
{
    int type = strtol(buf, NULL, 10) % NB_OF_MSG_TYPE;
    return type;
}

void create_msg(Msg *msg, char *buf)
{
    printf("buf : %s \n", buf);
    strtok(buf, ";");
    strcpy(msg->src.from, strtok(NULL, ";"));
    strcpy(msg->dst.to, strtok(NULL, ";"));
}

void send_msg(Msg msg, char *buf)
{
    struct Session *current = sessions;
    int found = 0;
    printf("Message for [%s]\n",msg.dst.to);

    while (current != NULL)
    {
        if (strcmp(current->username, msg.dst.to) == 0)
        {
            mg_ws_printf(current->c, WEBSOCKET_OP_TEXT, "%s", buf);
            printf("Message relayed\n");
            found = 1;
            break;
        }
        current = current->next;
    }

    if (!found)
    {
        printf("User %s offline. Saving to the DB...\n", msg.dst.to);
        db_save_msg(msg.src.from, msg.dst.to, buf);
    }
}

void say_hello(struct mg_connection *c, char *buf)
{
    struct Session *new = malloc(sizeof(struct Session));
    if (new == NULL) return;

    strtok(buf, ";");
    char *username = strtok(NULL, ";");
    if (username == NULL)
    {
        mg_ws_printf(c, WEBSOCKET_OP_TEXT, "%d;Bad format", ERR);
        free(new);
        return;
    }
    username[strcspn(username, "\r\n")] = '\0';

    new->c = c;
    strncpy(new->username, username, 23);
    new->username[23] = 0;
    new->authentificated = 0;
    generate_challenge(new->expected_challenge);
    printf("New session: [%s]", username);

    new->next = sessions;
    sessions = new;
    char hex_challenge[65];
    for (int i = 0; i < 32; i++)
        sprintf(hex_challenge + (i * 2), "%02x", sessions->expected_challenge[i]);
    mg_ws_printf(c, WEBSOCKET_OP_TEXT, "%d;%s", OK, hex_challenge);
}

void verify_auth(struct mg_connection *c, char *buf)
{
    strtok(buf, ";");
    char *user = strtok(NULL, ";");
    char *pubkey_hex = strtok(NULL, ";");
    char *sign_hex = strtok(NULL, ";");
    if (user == NULL || pubkey_hex == NULL || sign_hex == NULL)
    {
        mg_ws_printf(c, WEBSOCKET_OP_TEXT, "%d;Bad format", ERR);
        return;
    }

    if (user) user[strcspn(user, "\r\n")] = 0;
    if (pubkey_hex) ((char*)pubkey_hex)[strcspn((char*)pubkey_hex, "\r\n")] = 0;
    if (sign_hex) ((char*)sign_hex)[strcspn((char*)sign_hex, "\r\n")] = 0;

    struct Session *current = sessions;
    int found = 0;
    while (current != NULL)
    {
        if (strcmp(current->username, user) == 0)
        {
            found = 1;
            break;
        }
        current = current->next;
    }
    if (!found)
    {
        mg_ws_printf(c, WEBSOCKET_OP_TEXT, "%d;User doesn't exists", ERR);
        return;
    }

    uint8_t sig_bin[crypto_sign_BYTES];
    uint8_t pubkey_bin[crypto_sign_PUBLICKEYBYTES];
    if (sodium_hex2bin(pubkey_bin, sizeof(pubkey_bin), pubkey_hex, strlen(pubkey_hex), NULL, NULL, NULL) != 0 ||
        sodium_hex2bin(sig_bin, sizeof(sig_bin), sign_hex, strlen(sign_hex), NULL, NULL, NULL) != 0)
    {
        mg_ws_printf(c, WEBSOCKET_OP_TEXT, "%d;Invalid hex format", ERR);
        return;
    }
    // Juste avant crypto_eddsa_check
    printf("\n--- DEBUG AUTH ---\n");
    printf("User: %s\n", user);

    printf("Pubkey (hex attendue): ");
    for(int i=0; i<32; i++) printf("%02x", pubkey_bin[i]);

    printf("\nChallenge (hex attendu): ");
    for(int i=0; i<32; i++) printf("%02x", current->expected_challenge[i]);

    printf("\nSignature (hex reçue): %s\n", sign_hex);
    printf("DEBUG: Longueur sig_hex reçue: %zu\n", strlen((char*)sign_hex));
    printf("\n------------------\n");

    if (crypto_sign_verify_detached(sig_bin, current->expected_challenge, 32, pubkey_bin) == 0)
    {
        current->authentificated = 1;
        mg_ws_printf(c, WEBSOCKET_OP_TEXT, "%d;Authenticated", OK);
        db_send_pending(c, user);
        printf("Auth success for [%s]\n", user);
    }
    else
        mg_ws_printf(c, WEBSOCKET_OP_TEXT, "%d;Auth failed", ERR);
}

Msg init_msg(struct mg_connection* c, struct mg_ws_message *wm)
{
    Msg msg;
    char *buf = malloc(wm->data.len * sizeof(char) + 1);
    memset(buf, 0, wm->data.len);
    memcpy(buf, wm->data.buf, wm->data.len);
    buf[wm->data.len] = '\0';
    char *entireType = strtok(buf, ";");

    msg.type = get_type(entireType);

    char *buf2 = malloc(wm->data.len + 1);
    memcpy(buf2, wm->data.buf, wm->data.len);
    buf2[wm->data.len] = '\0';
    switch (msg.type)
    {
        case MSG:
            create_msg(&msg, buf2);
            memcpy(buf2, wm->data.buf, wm->data.len);
            buf2[wm->data.len] = '\0';
            send_msg(msg, buf2);
            break;
        case AUTH:
            verify_auth(c, buf2);
            break;
        case FIL:
            break;
        case BIN:
            break;
        case OK:
            break;
        case ERR:
            break;
        case CONNECT:
            say_hello(c, buf2);
            break;
    }
    printf("Type: %d\n", msg.type);
    free(buf);
    return msg;
}

void remove_session(struct mg_connection *c)
{
    struct Session **curr = &sessions;

    while (*curr) {
        struct Session *entry = *curr;
        if (entry->c == c) {
            printf("Déconnexion de l'utilisateur : [%s]\n", entry->username);
            *curr = entry->next;
            free(entry);
            return;
        }
        curr = &entry->next;
    }
}
