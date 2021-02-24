#include <lua5.3/lua.h>
#include <lua5.3/lualib.h>
#include <lua5.3/lauxlib.h>
#include <pthread.h>
#include <cstdlib>

/*
Compile it with `g++ -fPIC customThread.cpp -shared -lpthread -o customthread.so`
and load it using `require("customthread")`

WARNING:
1. There potential corruption when writing to a global variable at the same time (still working on mutex implementation to expose mutex to lua) or use `effil.G`
instead to share data between threads
2. Still experimental
3. Moving coroutine between thread is possible but not run to corutines in two or more threads
4. Only has 1 function returned by `require("customthread")` and it require 1 argument and thats its function to run on another thread
*/

struct WorkerArgument {
  lua_State* L;
};

void* worker(void* VAR) {
  WorkerArgument* arg = (WorkerArgument*) VAR;
  lua_call(arg->L, 0, 0);
  free(arg);
  return NULL;
}

int createThread(lua_State* L) {
  if (!lua_isfunction(L, -1)) {
    luaL_error(L, "Expect function at #1 argument");
  }
  
  lua_State* thr = lua_newthread(L);
  lua_pop(L, 1);
  lua_xmove(L, thr, 1);
  
  WorkerArgument* arg = (WorkerArgument*) malloc(sizeof(WorkerArgument));
  arg->L = thr;
  
  pthread_t fake;
  pthread_create(&fake, NULL, worker, (void*) arg);
  
  return 0;
}

extern "C" {
  int luaopen_customthread(lua_State* L) {
    lua_pushcfunction(L, createThread);
    return 1;
  }
}
