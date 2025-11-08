#include "mongoose.h"
#include <sqlite3.h>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>

#ifndef DEFAULT_DB_PATH
#define DEFAULT_DB_PATH "./sample.db"
#endif

class ProductHandler {
public:
    void sendProducts(struct mg_connection *c) {
        sqlite3 *db = nullptr;
        sqlite3_stmt *stmt = nullptr;
        const char *sql =
            "SELECT p.id, p.name, p.price, c.name "
            "FROM products p "
            "JOIN categories c ON p.category_id = c.id";

        if (sqlite3_open(DEFAULT_DB_PATH, &db) != SQLITE_OK) {
            mg_http_reply(c, 500, "", "{ \"error\": \"DB open failed\" }");
            return;
        }
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            mg_http_reply(c, 500, "", "{ \"error\": \"DB query failed\" }");
            sqlite3_close(db);
            return;
        }

        std::ostringstream buf;
        buf << "[";

        bool first = true;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            if (!first) buf << ",";
            first = false;
            int id = sqlite3_column_int(stmt, 0);
            const unsigned char *name = sqlite3_column_text(stmt, 1);
            double price = sqlite3_column_double(stmt, 2);
            const unsigned char *category = sqlite3_column_text(stmt, 3);
            buf << "{\"id\":" << id
                << ",\"name\":\"" << (name ? reinterpret_cast<const char*>(name) : "") << "\""
                << ",\"price\":" << price
                << ",\"category\":\"" << (category ? reinterpret_cast<const char*>(category) : "") << "\"}";
        }
        buf << "]";

        sqlite3_finalize(stmt);
        sqlite3_close(db);

        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", buf.str().c_str());
    }

    void sendProductById(struct mg_connection *c, int product_id) {
        sqlite3 *db = nullptr;
        sqlite3_stmt *stmt = nullptr;
        const char *sql =
            "SELECT p.id, p.name, p.price, c.name "
            "FROM products p "
            "JOIN categories c ON p.category_id = c.id "
            "WHERE p.id = ?";

        if (sqlite3_open(DEFAULT_DB_PATH, &db) != SQLITE_OK) {
            mg_http_reply(c, 500, "", "{ \"error\": \"DB open failed\" }");
            return;
        }
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            mg_http_reply(c, 500, "", "{ \"error\": \"DB query failed\" }");
            sqlite3_close(db);
            return;
        }

        sqlite3_bind_int(stmt, 1, product_id);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const unsigned char *name = sqlite3_column_text(stmt, 1);
            double price = sqlite3_column_double(stmt, 2);
            const unsigned char *category = sqlite3_column_text(stmt, 3);

            mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                "{\"id\":%d,\"name\":\"%s\",\"price\":%.2f,\"category\":\"%s\"}",
                id,
                name ? reinterpret_cast<const char*>(name) : "",
                price,
                category ? reinterpret_cast<const char*>(category) : "");
        } else {
            mg_http_reply(c, 404, "", "{ \"error\": \"Not found\" }");
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }
};

class Server {
public:
    Server() {
        mg_mgr_init(&mgr);
    }

    ~Server() {
        mg_mgr_free(&mgr);
    }

    void start() {
        if (mg_http_listen(&mgr, "http://0.0.0.0:8000", eventHandler, &handler) == nullptr) {
            std::cerr << "Error: listen failed" << std::endl;
            exit(1);
        }

        std::cout << "Server started on http://localhost:8000" << std::endl;
        for (;;) mg_mgr_poll(&mgr, 1000);
    }

private:
    static void eventHandler(struct mg_connection *c, int ev, void *ev_data) {
        if (ev == MG_EV_HTTP_MSG) {
            auto *hm = static_cast<struct mg_http_message *>(ev_data);

            if (hm->uri.len == strlen("/api/products") &&
                strncmp(hm->uri.buf, "/api/products", hm->uri.len) == 0) {
                handler.sendProducts(c);
            } else if (hm->uri.len > strlen("/api/products/") &&
                       strncmp(hm->uri.buf, "/api/products/", strlen("/api/products/")) == 0) {
                const char *p = hm->uri.buf + strlen("/api/products/");
                int id = std::atoi(p);
                handler.sendProductById(c, id);
            } else {
                struct mg_http_serve_opts opts = {.root_dir = "web"};
                mg_http_serve_dir(c, hm, &opts);
            }
        }
    }

    struct mg_mgr mgr;
    inline static ProductHandler handler;
};

int main() {
    Server server;
    server.start();
    return 0;
}