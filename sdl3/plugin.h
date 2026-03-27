#ifndef PLUGIN_H_
#define PLUGIN_H_

#include <SDL3/SDL.h>

typedef SDL_AppResult (*PluginInitFn)(void *);
typedef SDL_AppResult (*PluginLoopFn)(void *);
typedef SDL_AppResult (*PluginEventHandlerFn)(void **, SDL_Event *);
typedef void *(*PluginUnloadFn)(void);
typedef int (*PluginTerminateFn)(void);

typedef struct {
  char *filename;
  SDL_PathInfo path_info;
  SDL_SharedObject *dl_handle;
  
  void *state;

  PluginInitFn init;
  PluginLoopFn loop;
  PluginEventHandlerFn event_handler;
  PluginUnloadFn unload;
  PluginTerminateFn terminate;
} Plugin;

SDL_AppResult plugin_init(void *);
SDL_AppResult plugin_event_handler(void *, SDL_Event*);
SDL_AppResult plugin_loop(void *);
void *plugin_unload(void);
int plugin_terminate(void);

#ifdef PLUGIN_IMPLEMENTATION

#include <stdio.h>

int dl_plugin_open(Plugin *p) {
  p->dl_handle = SDL_LoadObject(p->filename);
  if (p->dl_handle == NULL) {
    fprintf(stderr, "ERROR: failed to open %s: %s\n", p->filename, SDL_GetError());
    return 1;
  }

  if (!SDL_GetPathInfo(p->filename, &p->path_info)) {
    SDL_Log("ERROR: failed getting path info %s", SDL_GetError());
    return 1;
  }
  
  printf("INFO: opened shared lib filename=%s, size=%ld, created=%ld, modified=%ld\n",
	 p->filename, p->path_info.size, p->path_info.create_time, p->path_info.modify_time);
  return 0;
}

int dl_plugin_close(Plugin p) {
  if (p.dl_handle == NULL) {
    return 0;
  }

  SDL_UnloadObject(p.dl_handle);
  printf("INFO: closed shared lib %s\n", p.filename);

  return 0;
}

void *dl_find_symbol(SDL_SharedObject *dl_handle, const char *symbol) {
  void *ref = SDL_LoadFunction(dl_handle, symbol);
  if (ref == NULL) {
    fprintf(stderr, "ERROR: failed finding symbol: %s\n", SDL_GetError());
    return NULL;
  }
  return ref;
}

void dl_plugin_symbols(Plugin *p) {
  p->init = dl_find_symbol(p->dl_handle, "plugin_init");
  p->loop = dl_find_symbol(p->dl_handle, "plugin_loop");
  p->event_handler = dl_find_symbol(p->dl_handle, "plugin_event_handler");
  p->unload = dl_find_symbol(p->dl_handle, "plugin_unload");
  p->terminate = dl_find_symbol(p->dl_handle, "plugin_terminate");
}

void dl_plugin_reload(Plugin *p) {
  dl_plugin_close(*p);
  dl_plugin_open(p);
  dl_plugin_symbols(p);
}


#endif // PLUGIN_IMPLEMENTATION
#endif // PLUGIN_H_
