#pragma once

#include "mongoose.h"

struct Session {
    struct mg_connection *c;
    char username[24];
    struct Session *next;
};
