#include "btmp.h"
#include "mongoose.h"
#include "db.h"


static void ev_handler(struct mg_connection *c, int ev, void *ev_data)
{
    if (ev == MG_EV_HTTP_MSG)
    {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        if (mg_match(hm->uri, mg_str("/ws"), NULL))
            mg_ws_upgrade(c, hm, NULL);
        else
            mg_http_reply(c, 200, "", "Serveur C Wilrakov prêt\n");
    }
    else if (ev == MG_EV_WS_MSG)
    {
        struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
        init_msg(c, wm);
        //printf("Reçu via WS : %s\n", wm->data.buf);
    }
    else if (ev == MG_EV_CLOSE)
    {
        remove_session(c);
    }
}

int main()
{
    struct mg_mgr mgr;
    db_init();
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://0.0.0.0:8000", ev_handler, NULL);
    for (;;)
        mg_mgr_poll(&mgr, 1000);

    mg_mgr_free(&mgr);
    return 0;
}
