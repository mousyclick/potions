#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include "plugin.h"

char* load_file_content(const char* filename) {
  FILE *fp;
  long size = 0;
  char* content;
    
  fp = fopen(filename, "rb");
  if(fp == NULL) {
    fprintf(stderr, "ERROR: failed to read file %s: %s\n", filename, strerror(errno));
    exit(1);
  }
  fseek(fp, 0L, SEEK_END);
  size = ftell(fp) + 1;
  fclose(fp);

  fp = fopen(filename, "r");
  content = memset(malloc(size), '\0', size);
  fread(content, 1, size - 1, fp);
  fclose(fp);

  return content;
}

GLuint compile_shader(const char* filename, GLenum shader_type) {
  GLuint shader = glCreateShader(shader_type);
  const char *shader_src = load_file_content(filename);
  glShaderSource(shader, 1, &shader_src, NULL);
  glCompileShader(shader);

  GLint compiled;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  
  if (compiled != GL_TRUE) {
    GLsizei log_length = 0;
    GLchar log_message[1024];
    glGetShaderInfoLog(shader, 1024, &log_length, log_message);
    fprintf(stderr, "Failed to compile shader %s\n", log_message);
    free((void *)shader_src);
    exit(1);
  }
  free((void *)shader_src);
  
  return shader;
}

GLuint link_program(GLuint vert_shader, GLuint frag_shader) {
  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, vert_shader);
  glAttachShader(shader_program, frag_shader);
  glLinkProgram(shader_program);

  GLint linked;
  glGetProgramiv(shader_program, GL_LINK_STATUS, &linked);

  if (linked != GL_TRUE) {
    GLsizei log_length = 0;
    GLchar log_message[1024];
    glGetProgramInfoLog(shader_program, 1024, &log_length, log_message);
    fprintf(stderr, "Failed to link program %s\n", log_message);
    exit(1);
  }

  return shader_program;
}

GLuint create_triangle() {
  float verts[] = {
    -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
  };
  
  GLuint VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  
  GLuint VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  
  glBindVertexArray(0);
  return VAO;
}

void draw_triangle(GLuint vao) {
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

// 0xRRGGBBAA 0-1
#define RGBAF(color)				\
  (((color) >> 8 * 3) & 0xFF) / (float) 0xFF,	\
    (((color) >> 8 * 2) & 0xFF) / (float) 0xFF,	\
    (((color) >> 8 * 1) & 0xFF) / (float) 0xFF,	\
    (((color) >> 8 * 0) & 0xFF) / (float) 0xFF

typedef struct {
  size_t size;
  SDL_Window *window;
  SDL_GLContext gl_context;
  GLuint triangleVao;
  GLuint shader_program;
  GLuint utime_loc;
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
      return SDL_APP_FAILURE;
    } else if (size > old_state->size) {
      printf("INFO: new state %zu bytes > oldstate %zu bytes\n", size,
             old_state->size);
      state = realloc(state, size);
    } else {
      printf("INFO: no state size changes\n");
      state = data;
    }

    // TODO: investigate if GLAD has same issue when unloading shared lib of unbinding gl functions
    GLenum err = glewInit();
    if (GLEW_OK != err) {
      fprintf(stderr, "ERROR: couldn't initialize GLEW: %s\n", glewGetErrorString(err));
      return 1;
    }
  } else {
    printf("INFO: no old state, creating new one\n");
    size_t size = sizeof(PluginState);
    state = malloc(size);
    state->size = size;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
      SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
      return SDL_APP_FAILURE;
    }

    state->window = SDL_CreateWindow("SDL3 + OpenGL (Triangle)", 1600, 900, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!state->window) {
      SDL_Log("Couldn't create SDL+OpenGL window: %s", SDL_GetError());
      return SDL_APP_FAILURE;
    }
    SDL_SetWindowPosition(state->window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    state->gl_context = SDL_GL_CreateContext(state->window);

    GLenum err = glewInit();
    if (GLEW_OK != err) {
      fprintf(stderr, "ERROR: couldn't initialize GLEW: %s\n", glewGetErrorString(err));
      return 1;
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    GLint gl_major, gl_minor;
    glGetIntegerv(GL_MAJOR_VERSION, &gl_major);
    glGetIntegerv(GL_MINOR_VERSION, &gl_minor);
    
    SDL_Log("OpenGL version %d.%d", gl_major, gl_minor);

    state->triangleVao = create_triangle();

    GLuint vert_shader = compile_shader("tri.vs", GL_VERTEX_SHADER);
    GLuint frag_shader = compile_shader("tri.fs", GL_FRAGMENT_SHADER);
    state->shader_program = link_program(vert_shader, frag_shader);
    glDeleteShader(frag_shader);
    glDeleteShader(vert_shader);

    state->utime_loc = glGetUniformLocation(state->shader_program, "u_time");
  }
  return SDL_APP_CONTINUE;
}

void *plugin_unload(void) {
  printf("DEBUG: unloading plugin\n");
  return state;
}

SDL_AppResult plugin_event_handler(void *appstate, SDL_Event *e) {
  (void)appstate;

  switch (e->type) {
  case SDL_EVENT_WINDOW_RESIZED:
    glViewport(0, 0, e->window.data1, e->window.data2);
    return SDL_APP_CONTINUE;
    break;
  }
    
  return SDL_APP_CONTINUE;
}

SDL_AppResult plugin_loop(void *appstate) {
  (void) appstate;

  const double now = ((double)SDL_GetTicks()) / 1000.0;

  glClearColor(RGBAF(0x330000FF));
  glClear(GL_COLOR_BUFFER_BIT);
  
  glUseProgram(state->shader_program);
  glUniform1f(state->utime_loc, now);
  draw_triangle(state->triangleVao);

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
