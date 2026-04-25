#include <stdio.h>
#include "btmp.h"
#include "mongoose.h"
#include "network.h"

#define NB_OF_MSG_TYPE 7

struct Session *sessions = NULL;

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
        return;
    }
    username[strcspn(username, "\r\n")] = '\0';

    new->c = c;
    strncpy(new->username, username, 23);
    new->username[23] = 0;
    new->next = sessions;
    sessions = new;
    printf("New session: [%s]", username);
    mg_ws_printf(c, WEBSOCKET_OP_TEXT, "%d;Welcome to Wilo Chat %s!", OK, username);
}

Msg init_msg(struct mg_connection* c, struct mg_ws_message *wm)
{
    Msg msg;
    char *buf = malloc(wm->data.len * sizeof(char));
    memset(buf, 0, wm->data.len);
    strcpy(buf, wm->data.buf);
    char *entireType = strtok(buf, ";");

    msg.type = get_type(entireType);
    strcpy(buf, wm->data.buf);
    switch (msg.type)
    {
        case MSG:
            create_msg(&msg, buf);
            strcpy(buf, wm->data.buf);
            send_msg(msg, buf);
            break;
        case CREATE:
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
            say_hello(c, buf);
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
