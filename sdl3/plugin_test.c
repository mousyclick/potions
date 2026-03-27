#include <stdio.h>
#include <stdlib.h>

#include <SDL3/SDL.h>
#include "plugin.h"

// 0xAABBGGRR
#define UNWRAP(color)				\
  (color >> 8 * 0) & 0xFF,			\
    (color >> 8 * 1) & 0xFF,			\
    (color >> 8 * 2) & 0xFF,			\
    (color >> 8 * 3) & 0xFF

typedef struct {
  size_t size;
  SDL_Window *window;
  SDL_Renderer *renderer;
} PluginState;

static PluginState *state;

SDL_AppResult plugin_init(void *data) {
  if (data != NULL) {
    PluginState *old_state = data;
    printf("INFO: restoring old state %zu bytes\n", old_state->size);
    size_t size = sizeof(PluginState);
    if (size < old_state->size) {
      fprintf(stderr, "ERROR: sizeof state %zu bytes < oldstate %zu bytes\n",
              size, old_state->size);
    } else if (size > old_state->size) {
      printf("INFO: new state %zu bytes > oldstate %zu bytes\n", size,
             old_state->size);
      state = realloc(state, size);
    }
    state = data;
  } else {
    printf("INFO: no old state, creating new one\n");
    size_t size = sizeof(PluginState);
    state = malloc(size);
    state->size = size;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
      SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
      return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Hello SDL3", 1600, 900, 0, &state->window, &state->renderer)) {
      SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
      return SDL_APP_FAILURE;
    }
  }
  return SDL_APP_CONTINUE;
}

void *plugin_unload(void) {
  printf("DEBUG: unloading plugin\n");
  return state;
}

SDL_AppResult plugin_event_handler(void *appstate, SDL_Event *event) {
  (void)appstate;
  (void)event;
  return SDL_APP_CONTINUE;
}

SDL_AppResult plugin_loop(void *appstate) {
  (void) appstate;

  SDL_Renderer *renderer = state->renderer;
  
  const double now = ((double)SDL_GetTicks()) / 1000.0;
  const float red = (float) (0.5 + 0.5 * SDL_sin(now));
  const float green = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 2 / 3));
  const float blue = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 4 / 3));
  SDL_SetRenderDrawColorFloat(renderer, red, green, blue, SDL_ALPHA_OPAQUE_FLOAT);
  //SDL_SetRenderDrawColor(renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);


  SDL_SetRenderDrawColor(renderer, UNWRAP(0xFFBB0000));
  SDL_RenderDebugTextFormat(renderer, 10, 10, "Hello %s", ", World");
  
  SDL_RenderPresent(renderer);
  return SDL_APP_CONTINUE;
}

int plugin_terminate(void) {
  printf("DEBUG: terminating plugin\n");
  SDL_DestroyRenderer(state->renderer);
  SDL_DestroyWindow(state->window);
  free(state);
  return 0;
}
