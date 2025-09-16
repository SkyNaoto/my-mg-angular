#include "mongoose.h"
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DB_FILE "sample.db"

// ----------------- 全件取得 -----------------
static void send_products(struct mg_connection *c) {
  sqlite3 *db = NULL;
  sqlite3_stmt *stmt = NULL;
  const char *sql =
    "SELECT p.id, p.name, p.price, c.name "
    "FROM products p "
    "JOIN categories c ON p.category_id = c.id";

  if (sqlite3_open(DB_FILE, &db) != SQLITE_OK) {
    mg_http_reply(c, 500, "", "{ \"error\": \"DB open failed\" }");
    return;
  }
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    mg_http_reply(c, 500, "", "{ \"error\": \"DB query failed\" }");
    sqlite3_close(db);
    return;
  }

  char buf[8192];
  size_t len = 0;
  len += snprintf(buf + len, sizeof(buf) - len, "[");

  int first = 1;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    if (!first) len += snprintf(buf + len, sizeof(buf) - len, ",");
    first = 0;
    int id = sqlite3_column_int(stmt, 0);
    const unsigned char *name = sqlite3_column_text(stmt, 1);
    double price = sqlite3_column_double(stmt, 2);
    const unsigned char *category = sqlite3_column_text(stmt, 3);
    len += snprintf(buf + len, sizeof(buf) - len,
      "{\"id\":%d,\"name\":\"%s\",\"price\":%.2f,\"category\":\"%s\"}",
      id,
      name ? (const char*)name : "",
      price,
      category ? (const char*)category : ""
    );
  }
  len += snprintf(buf + len, sizeof(buf) - len, "]");

  sqlite3_finalize(stmt);
  sqlite3_close(db);

  mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", buf);
}

// ----------------- 1件取得 -----------------
static void send_product_by_id(struct mg_connection *c, int product_id) {
  sqlite3 *db = NULL;
  sqlite3_stmt *stmt = NULL;
  const char *sql =
    "SELECT p.id, p.name, p.price, c.name "
    "FROM products p "
    "JOIN categories c ON p.category_id = c.id "
    "WHERE p.id = ?";

  if (sqlite3_open(DB_FILE, &db) != SQLITE_OK) {
    mg_http_reply(c, 500, "", "{ \"error\": \"DB open failed\" }");
    return;
  }
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
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
      name ? (const char*)name : "",
      price,
      category ? (const char*)category : ""
    );
  } else {
    mg_http_reply(c, 404, "", "{ \"error\": \"Not found\" }");
  }

  sqlite3_finalize(stmt);
  sqlite3_close(db);
}

// ----------------- イベントハンドラ -----------------
static void fn(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;

    // API
    if (hm->uri.len == strlen("/api/products") &&
        strncmp(hm->uri.buf, "/api/products", hm->uri.len) == 0) {
      send_products(c);
    } else if (hm->uri.len > strlen("/api/products/") &&
               strncmp(hm->uri.buf, "/api/products/", strlen("/api/products/")) == 0) {
      const char *p = hm->uri.buf + strlen("/api/products/");
      int id = atoi(p);
      send_product_by_id(c, id);
    } else {
      // Angular アプリの静的ファイルを配信
      struct mg_http_serve_opts opts = {.root_dir = "web"};
      mg_http_serve_dir(c, hm, &opts);
    }
  }
}

// ----------------- メイン関数 -----------------
int main(void) {
  struct mg_mgr mgr;
  mg_mgr_init(&mgr);

  if (mg_http_listen(&mgr, "http://0.0.0.0:8000", fn, NULL) == NULL) {
    fprintf(stderr, "Error: listen failed\n");
    return 1;
  }

  printf("Server started on http://localhost:8000\n");
  for (;;) mg_mgr_poll(&mgr, 1000);

  mg_mgr_free(&mgr);
  return 0;
}
