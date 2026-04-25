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
    CONNECT = 6,    // Say hello to the server
} MsgType;

typedef struct {
    MsgType type;
    union {
        char from[24];
        char name[24];
    } src;
    union {
        char to[24];
        char email[24];
    } dst;
    char hash[64];
    char sign[64];
    int msgLen;
    char *msg;
} Msg;

/*
 * ======================
 * TYPE: MSG
 * FROM: wilrak0v
 * TO: makroHard
 * HASH: ghqsuflhelqgqs
 * SIGN: <sign-base64>
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
 * HASH: mqsghdflkhsamq     // Décrypter (clé publique)
 * PASSWD: <passwd>         // Encrypter (clé privée)
 *
 * TYPE: ERR
 * SIGN: <sign-base64>
 * LEN: <len>
 * MSG: <msg>
 *
 * ======================
 * TYPE: CONNECT            // Signaler qu'on est connecté pour recevoir les messages en attentes
 * NAME: wilrak0v
 * HASH: sqmghmqlghkl
 */
