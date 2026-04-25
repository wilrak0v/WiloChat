#include <stdio.h>
#include "btmp.h"
#include "mongoose.h"

#define NB_OF_MSG_TYPE 7

Msg init_msg(struct mg_ws_message *wm)
{
    Msg msg;
    char *buf = malloc(wm->data.len * sizeof(char));
    memset(buf, 0, wm->data.len);
    strcpy(buf, wm->data.buf);
    char *entireType = strtok(buf, ";");

    strtok(entireType, ":");
    entireType = strtok(NULL, ":");
    int type = strtol(entireType, NULL, 10) % NB_OF_MSG_TYPE;
    msg.type = type;
    printf("Type: %d\n", type);
    return msg;
}
