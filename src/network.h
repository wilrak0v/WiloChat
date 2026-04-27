#pragma once

#include "mongoose.h"
#include <stdint.h>

struct Session {
    struct mg_connection *c;
    char username[24];
    uint8_t expected_challenge[32];
    int authentificated;
    struct Session *next;
};
