#include <stdio.h>
#include <stdlib.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include "plugin.h"

// 0xRRGGBBAA 0-1
#define RGBAF(color)                              \
  (((color) >> 8 * 0) & 0xFF) / (float) 0xFF,     \
    (((color) >> 8 * 1) & 0xFF) / (float) 0xFF,   \
    (((color) >> 8 * 2) & 0xFF) / (float) 0xFF,   \
    (((color) >> 8 * 3) & 0xFF) / (float) 0xFF

typedef struct {
  size_t size;
  SDL_Window *window;
  SDL_GLContext gl_context;
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

    state->window = SDL_CreateWindow("Hello SDL3 + OpenGL", 1600, 900, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!state->window) {
      SDL_Log("Couldn't create SDL+OpenGL window: %s", SDL_GetError());
      return SDL_APP_FAILURE;
    }
    SDL_SetWindowPosition(state->window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    state->gl_context = SDL_GL_CreateContext(state->window);
    GLint gl_major, gl_minor;
    glGetIntegerv(GL_MAJOR_VERSION, &gl_major);
    glGetIntegerv(GL_MINOR_VERSION, &gl_minor);
    
    SDL_Log("OpenGL version %d.%d", gl_major, gl_minor);
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

  const double now = ((double)SDL_GetTicks()) / 1000.0;
  const float red = (float) (0.5 + 0.5 * SDL_sin(now));
  const float green = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 2 / 3));
  const float blue = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 4 / 3));
  
  glClearColor(red, green, blue, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  SDL_GL_SwapWindow(state->window);
  return SDL_APP_CONTINUE;
}

int plugin_terminate(void) {
  printf("DEBUG: terminating plugin\n");
  SDL_GL_DestroyContext(state->gl_context);
  SDL_DestroyWindow(state->window);
  free(state);
  return 0;
}
