#include "db.h"
#include "mongoose.h"
#include <sqlite3.h>
#include <stdio.h>

sqlite3 *db;

void db_init()
{
    int rc = sqlite3_open("social.db", &db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(db));
        return;
    }

    const char *sql = "CREATE TABLE IF NOT EXISTS messages ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "src TEXT,"
                      "dst TEXT,"
                      "content TEXT,"
                      "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";

    char *err_msg = NULL;
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
}

void db_save_msg(const char *src, const char *dst, const char *content)
{
    sqlite3_stmt *res;
    const char *sql = "INSERT INTO messages (src, dst, content) VALUES (?,?,?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(res, 1, src, -1, SQLITE_STATIC);
    sqlite3_bind_text(res, 2, dst, -1, SQLITE_STATIC);
    sqlite3_bind_text(res, 3, content, -1, SQLITE_STATIC);

    rc = sqlite3_step(res);
    if (rc != SQLITE_DONE)
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(db));

    sqlite3_finalize(res);
    printf("Message saved\n");
}

void db_send_pending(struct mg_connection *c, const char *username)
{
    sqlite3_stmt *res;
    const char *sql = "SELECT src, content FROM messages WHERE dst = ? ORDER BY timestamp ASC;";
    if (sqlite3_prepare_v2(db, sql, -1, &res, 0) != SQLITE_OK) return;

    sqlite3_bind_text(res, 1, username, -1, SQLITE_STATIC);
    printf("Send messages for [%s]...\n", username);

    while (sqlite3_step(res) == SQLITE_ROW)
    {
        const char *content = (const char *) sqlite3_column_text(res, 1);
        mg_ws_printf(c, WEBSOCKET_OP_TEXT, "%s", content);
    }
    sqlite3_finalize(res);

    char *del_sql = sqlite3_mprintf("DELETE FROM messages WHERE dst = %Q", username);
    sqlite3_exec(db, del_sql, 0, 0, 0);
    sqlite3_free(del_sql);
}
