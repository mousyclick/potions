#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define PLUGIN_IMPLEMENTATION
#include "plugin.h"

#define FPS 60
#define MSPF 1000 / FPS

static Plugin plugin;
static bool plugin_updated = false;

Uint32 plugin_monitor_callback(void *data, SDL_TimerID timer_id, Uint32 interval) {
  (void)data;
  (void)timer_id;

  SDL_PathInfo info;
  SDL_GetPathInfo(plugin.filename, &info);
  if (info.create_time != plugin.path_info.create_time) {
    SDL_Log("Plugin was updated");
    plugin.path_info.create_time = info.create_time;
    plugin_updated = true;
  }
  
  return interval;
}

void reload_plugin() {
  SDL_Log("INFO: plugin_updated = true\n");
  plugin.state = plugin.unload();
  dl_plugin_reload(&plugin);
  plugin.init(plugin.state);
  plugin_updated = false;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char* argv[]) {
  (void)appstate;
  (void)argc;
  (void)argv;
  
  SDL_SetAppMetadata("Hello SDL3", "1.0", "domain.mousyclick");
  
  plugin.filename = "build/plugin_gl_cube.so";

  dl_plugin_reload(&plugin);

  Uint32 timer_id = SDL_AddTimer(1000, plugin_monitor_callback, NULL);
  (void)timer_id;
  return plugin.init(NULL);
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *e) {
  (void)appstate;

  if (e->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  }

  if (e->type == SDL_EVENT_KEY_DOWN) {
    if (e->key.key == SDLK_R) {
      if (e->key.mod & SDL_KMOD_SHIFT) {
        plugin.state = NULL;
        plugin.terminate();
        dl_plugin_reload(&plugin);
        plugin.init(plugin.state);
      } else {
        plugin_updated = true;
      }
    }
  }
  
  return plugin.event_handler(appstate, e);
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  (void)appstate;

  if (plugin_updated) {
    reload_plugin();
    return SDL_APP_CONTINUE;
  }
  
  const Uint32 start = SDL_GetTicks();
  
  plugin.loop(appstate);

  const Uint32 duration = SDL_GetTicks() - start;
  if (duration < MSPF) {
    SDL_Delay(MSPF - duration);
  }

  
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  (void)appstate;
  (void)result;
  plugin.terminate();
}
