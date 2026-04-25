#pragma once

#include <sqlite3.h>
#include "mongoose.h"

void db_init();
void db_save_msg(const char *src, const char *dst, const char *content);
void db_send_pending(struct mg_connection *c, const char *username);
