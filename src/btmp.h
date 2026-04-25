/* btmp.h - wilrak0v, the 25th april 2026
 *
 */

#pragma once

typedef enum {
    MSG     = 0,    // Message
    CREATE  = 1,    // Create an account
    FIL     = 2,    // Send files
    BIN     = 3,    // Send binaries
    OK      = 4,    // OK
    ERR     = 5,    // An error
} MsgType;

union MultipleStrings {
    char from[24];
    char name[24];
    char to[24];
    char email[24];
};

typedef struct {
    MsgType type;
    union MultipleStrings fromAndname;
    union MultipleStrings toAndemail;
    char hash[256];
    int msgLen;
    char *msg;
} Msg;

/*
 * ======================
 * TYPE: MSG
 * FROM: wilrak0v
 * TO: makroHard
 * HASH: ghqsuflhelqgqs
 * LEN: <len>
 * MSG: <msg>
 *
 * ======================
 * TYPE: CREATE
 * NAME: wilrak0v
 * EMAIL: wilrakov@email.com
 * HASH: gmhdmuahodqsli
 *
 * TYPE: OK
 *
 * TYPE: ERR
 * LEN: <len>
 * MSG: <msg>
 */
