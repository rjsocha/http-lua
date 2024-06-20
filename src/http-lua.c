#define EPOLL
#define HTTPSERVER_IMPL
#include "httpserver.h"
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

const char luaScript[] __attribute__((section("LUAScript"))) = "local result = { response = 'OK', code = 200 }; return result";

char *toLower(char *p) {
  char *s = p;
  for ( ; *p; ++p) *p = tolower(*p);
  return (s);
}

void handle_request(struct http_request_s* request) {
  int iter = 0, i = 0;
  http_string_t key, val;
  http_string_t url = http_request_target(request);
  http_string_t method = http_request_method(request);
  char *workstr;

  http_request_connection(request, HTTP_CLOSE);
  struct http_response_s* response = http_response_init();

  lua_State *L = luaL_newstate();
  luaL_openlibs(L);

  lua_newtable(L);

  lua_pushstring(L, "url");
  workstr = strndup(url.buf, url.len);
  lua_pushstring(L, workstr);
  free(workstr);
  lua_settable(L, -3);

  lua_pushstring(L, "method");
  workstr = strndup(method.buf, method.len);
  lua_pushstring(L, workstr);
  free(workstr);
  lua_settable(L, -3);

  lua_pushstring(L, "headers");
  lua_newtable(L);

  while (http_request_iterate_headers(request, &key, &val, &iter)) {
    workstr = strndup(val.buf,val.len);
    lua_pushstring(L, workstr );
    free(workstr);
    workstr = toLower(strndup(key.buf,key.len));
    lua_setfield(L, -2, workstr);
    free(workstr);
  }
  lua_settable(L, -3);
  lua_setglobal(L, "request");

  http_response_header(response, "Content-Type", "text/plain");

  int result = luaL_dostring(L, luaScript);

  if( result == LUA_OK && lua_istable(L, -1) ) {
    lua_getfield(L, -1, "response");
    const char *body = lua_tostring(L, -1);
    fprintf(stderr, "RESPONSE(%ld): %s\n", strlen(body),body);
    lua_pop(L, 1);
    lua_getfield(L, -1, "code");
    int code = (int)lua_tointeger(L, -1);
    lua_pop(L, 1);
    http_response_status(response, code);
    http_response_body(response, body, strlen(body));
  } else {
    fprintf(stderr, "Error: %s\n", lua_tostring(L, -1));
  }
  lua_close(L);
  http_respond(request, response);
}

struct http_server_s* server;

void handle_sigterm(int signum) {
  free(server);
  exit(0);
}

int main() {
int port;
  port=8000;
  server = http_server_init(port, handle_request);
  signal(SIGTERM, handle_sigterm);
  signal(SIGINT, handle_sigterm);
  http_server_listen(server);
  return 1;
}
