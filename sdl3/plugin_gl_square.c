#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <cglm/cglm.h>

#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include "plugin.h"

#define WIDTH 1600
#define HEIGHT 900

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

GLuint load_texture(const char* filename, GLint internal_format, GLenum format) {
  GLuint id;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  stbi_set_flip_vertically_on_load(true);  
  int width, height, channels;
  unsigned char *data = stbi_load(filename, &width, &height, &channels, 0);
  
  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(data);
  return id;
}

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

GLuint create_square() {
  float verts[] = {
    // pos colors uv
    0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
    0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
    -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f // top left
  };

  int indices[] = {
    0, 1, 3,
    1, 2, 3
  };

  GLuint VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  
  GLuint VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

  GLuint EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);
  return VAO;
}

void draw_square(GLuint vao) {
  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// 0xRRGGBBAA 0-1
#define RGBAF(color)				\
  (((color) >> 8 * 3) & 0xFF) / (float) 0xFF,	\
    (((color) >> 8 * 2) & 0xFF) / (float) 0xFF,	\
    (((color) >> 8 * 1) & 0xFF) / (float) 0xFF,	\
    (((color) >> 8 * 0) & 0xFF) / (float) 0xFF

typedef struct {
  size_t size;
  int width;
  int height;
  float aspect;
  SDL_Window *window;
  SDL_GLContext gl_context;
  GLuint square_vao;
  GLuint shader_program;
  GLuint u_time;
  GLuint u_transform;
  GLuint u_model;
  GLuint u_view;
  GLuint u_projection;
  GLuint container_tex;
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
    state->width = WIDTH;
    state->height = HEIGHT;
    state->aspect = WIDTH / (float) HEIGHT;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
      SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
      return SDL_APP_FAILURE;
    }

    state->window = SDL_CreateWindow("SDL3 + OpenGL (Square)", state->width, state->height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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

    state->square_vao = create_square();

    GLuint vert_shader = compile_shader("assets/square.vs", GL_VERTEX_SHADER);
    GLuint frag_shader = compile_shader("assets/square.fs", GL_FRAGMENT_SHADER);
    state->shader_program = link_program(vert_shader, frag_shader);
    glDeleteShader(frag_shader);
    glDeleteShader(vert_shader);

    state->container_tex = load_texture("assets/container.jpg", GL_RGB, GL_RGB);

    GLuint u_texture = glGetUniformLocation(state->shader_program, "u_texture");
    glUniform1i(u_texture, 0); // TODO: figure out why this is necessary
    state->u_time = glGetUniformLocation(state->shader_program, "u_time");
    state->u_transform = glGetUniformLocation(state->shader_program, "u_transform");

    state->u_model = glGetUniformLocation(state->shader_program, "u_model");
    state->u_view = glGetUniformLocation(state->shader_program, "u_view");
    state->u_projection = glGetUniformLocation(state->shader_program, "u_projection");
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
  case SDL_EVENT_KEY_DOWN:
    switch (e->key.key) {
    case SDLK_ESCAPE:
      SDL_Event event = { SDL_EVENT_QUIT };
      SDL_PushEvent(&event);
      break;
    }
    break; // SDL_EVENT_KEY_DOWN
  case SDL_EVENT_WINDOW_RESIZED:
    int w = e->window.data1;
    int h = e->window.data2;
    glViewport(0, 0, w, h);
    state->width = w;
    state->height = h;
    state->aspect = w / (float) h;
    return SDL_APP_CONTINUE;
    break; // SDL_EVENT_WINDOW_RESIZED
  }
    
  return SDL_APP_CONTINUE;
}

SDL_AppResult plugin_loop(void *appstate) {
  (void) appstate;

  const double now = ((double)SDL_GetTicks()) / 1000.0;

  glClearColor(RGBAF(0x330000FF));
  glClear(GL_COLOR_BUFFER_BIT);

  vec4 vec = {1.0f, 0.0f, 0.0f, 1.0f};
  mat4 trans; glm_mat4_identity(trans);
  glm_rotate_x(trans, glm_rad(-55.0f), trans);
  glm_rotate_z(trans, glm_rad(sin(now) * 90.0f), trans);
  glm_scale(trans, (vec3){0.5f, 0.5f, 0.5f});
  glm_translate(trans, (vec3){cosf(now), sinf(now), 0.0f});
  glm_mat4_mulv(trans, vec, vec);

  mat4 model; glm_mat4_identity(model);
  //glm_rotate(model, glm_rad(-55.0f), (vec3){1.0f, 0.0f, 0.0f});
  mat4 view; glm_mat4_identity(view);
  glm_translate(view, (vec3){0.0f, 0.0f, sin(now) * 1.0f - 3.0f});
  mat4 projection;
  glm_perspective(glm_rad(45.0f), state->aspect, 0.1f, 100.0f, projection);
  
  glUseProgram(state->shader_program);
  glUniform1f(state->u_time, now);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, state->container_tex);
  glUniformMatrix4fv(state->u_transform, 1, GL_FALSE, (float *)trans);
  glUniformMatrix4fv(state->u_model, 1, GL_FALSE, (float *)model);
  glUniformMatrix4fv(state->u_view, 1, GL_FALSE, (float *)view);
  glUniformMatrix4fv(state->u_projection, 1, GL_FALSE, (float *)projection);
  draw_square(state->square_vao);

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
