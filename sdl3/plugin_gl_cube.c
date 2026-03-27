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
#define GRID_SIZE 10

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
    fprintf(stderr, "ERROR: Failed to compile shader %s: %s\n", filename, log_message);
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

GLuint create_program(const char *vertex_shader_filename, const char *fragment_shader_filename) {
    GLuint vert_shader = compile_shader(vertex_shader_filename, GL_VERTEX_SHADER);
    GLuint frag_shader = compile_shader(fragment_shader_filename, GL_FRAGMENT_SHADER);
    GLuint shader_program = link_program(vert_shader, frag_shader);
    glDeleteShader(frag_shader);
    glDeleteShader(vert_shader);
    return shader_program;
}

GLuint create_cube() {
  float verts[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
  };
  
  GLuint VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  
  GLuint VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
  return VAO;
}

void draw_cube(GLuint vao) {
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 36);
}

GLuint create_grid() {
  const int size = GRID_SIZE;
  float verts[GRID_SIZE * GRID_SIZE * 4] = {0};

  int i = 0;
  for (int row = -size; row < size; row++) {
    verts[i++] = -size; // x1
    verts[i++] = row; // y1
    verts[i++] = size; // x2
    verts[i++] = row; // y2
  }

  for (int col = -size; col < size; col++) {
    verts[i++] = col; // x1
    verts[i++] = -size; // y1
    verts[i++] = col; // x2
    verts[i++] = size; // y2
  }
  
  GLuint VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  
  GLuint VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
  return VAO;
}

void draw_grid(GLuint vao) {
  glBindVertexArray(vao);
  glDrawArrays(GL_LINES, 0, (GRID_SIZE * GRID_SIZE));
}

GLuint create_axes() {
  float verts[] = {
    -GRID_SIZE, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f,
    GRID_SIZE, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,

    0.0f, -GRID_SIZE, 0.0f, 0.0f, 0.0f, 0.2f,
    0.0f, GRID_SIZE, 0.0f, 0.0f, 0.0f, 1.0f,

    0.0f, 0.0f, -GRID_SIZE, 0.2f, 0.0f, 0.0f,
    0.0f, 0.0f, GRID_SIZE, 1.0f, 0.0f, 0.0f,    
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

void draw_axes(GLuint vao) {
  glBindVertexArray(vao);
  glDrawArrays(GL_LINES, 0, 6);
}


// 0xRRGGBBAA 0-1
#define RGBAF(color)				\
  (((color) >> 8 * 3) & 0xFF) / (float) 0xFF,	\
    (((color) >> 8 * 2) & 0xFF) / (float) 0xFF,	\
    (((color) >> 8 * 1) & 0xFF) / (float) 0xFF,	\
    (((color) >> 8 * 0) & 0xFF) / (float) 0xFF

typedef struct {
  vec3 position;
  vec3 target;
  vec3 direction;
  vec3 right;
  vec3 up;
  mat4 view;
} Camera;

typedef struct {
  GLuint vao;
  GLuint u_model;
  GLuint u_view;
  GLuint u_projection;
  GLuint program;
  
  GLuint u_time;
  GLuint u_texture;
  GLuint u_texture2;
  
  GLuint texture;
  GLuint texture2;
} Cube;

typedef struct {
  GLuint vao;
  GLuint u_model;
  GLuint u_view;
  GLuint u_projection;
  GLuint program;
} Grid;

typedef struct {
  GLuint vao;
  GLuint u_model;
  GLuint u_view;
  GLuint u_projection;
  GLuint program;
} Axes;

typedef struct {
  size_t size;
  int width;
  int height;
  float aspect;
  float delta_time;
  double last_frame_time;
  SDL_Window *window;
  SDL_GLContext gl_context;
  Camera camera;
  
  Cube cube;
  Grid grid;
  Axes axes;
} PluginState;

static PluginState *state;

void update_camera(Camera *camera) {
  glm_vec3_sub(camera->position, camera->target, camera->direction);
  glm_vec3_normalize(camera->direction);

  vec3 up = {0.0f, 1.0f, 0.0f};
  glm_vec3_cross(up, camera->direction, camera->right);
  glm_vec3_cross(camera->direction, camera->right, camera->up);

  glm_lookat(camera->position, camera->target, camera->up, camera->view);
}

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

    state->window = SDL_CreateWindow("SDL3 + OpenGL (Cube)", state->width, state->height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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
    glEnable(GL_DEPTH_TEST);

    GLint gl_major, gl_minor;
    glGetIntegerv(GL_MAJOR_VERSION, &gl_major);
    glGetIntegerv(GL_MINOR_VERSION, &gl_minor);
    
    SDL_Log("OpenGL version %d.%d", gl_major, gl_minor);


    state->cube.program = create_program("assets/cube.vs", "assets/cube.fs");
    glUseProgram(state->cube.program);
    state->cube.u_model = glGetUniformLocation(state->cube.program, "u_model");
    state->cube.u_view = glGetUniformLocation(state->cube.program, "u_view");
    state->cube.u_projection = glGetUniformLocation(state->cube.program, "u_projection");
    state->cube.u_time = glGetUniformLocation(state->cube.program, "u_time");
    state->cube.texture = load_texture("assets/container.jpg", GL_RGB, GL_RGB);
    state->cube.texture2 = load_texture("assets/mousyclick.png", GL_RGBA, GL_RGBA);
    state->cube.u_texture = glGetUniformLocation(state->cube.program, "u_texture");
    glUniform1i(state->cube.u_texture, 0);
    state->cube.u_texture2 = glGetUniformLocation(state->cube.program, "u_texture2");
    glUniform1i(state->cube.u_texture2, 1);
    state->cube.vao = create_cube();
    
    state->grid.program = create_program("assets/grid.vs", "assets/grid.fs");
    glUseProgram(state->grid.program);
    state->grid.u_model = glGetUniformLocation(state->grid.program, "u_model");
    state->grid.u_view = glGetUniformLocation(state->grid.program, "u_view");
    state->grid.u_projection = glGetUniformLocation(state->grid.program, "u_projection");
    state->grid.vao = create_grid();
    
    state->axes.program = create_program("assets/axes.vs", "assets/axes.fs");
    glUseProgram(state->grid.program);
    state->axes.u_model = glGetUniformLocation(state->axes.program, "u_model");
    state->axes.u_view = glGetUniformLocation(state->axes.program, "u_view");
    state->axes.u_projection = glGetUniformLocation(state->axes.program, "u_projection");
    state->axes.vao = create_axes();

    glUseProgram(0);
    
    Camera camera = {
      .position = {0.0f, 1.0f, 5.0f},
      .target = {0.0f, 0.0f, 0.0f}
    };
    state->camera = camera;
    update_camera(&state->camera);

    state->delta_time = 16.666f;
    state->last_frame_time = (double) SDL_GetTicks() / 1000.0;
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

void handle_keyboard() {
  const bool *keyboard_state = SDL_GetKeyboardState(NULL);
  const float camera_speed = 0.2f;
  vec3 move;
  
  if (keyboard_state[SDL_SCANCODE_W]) {
    glm_vec3_scale(state->camera.direction, -camera_speed, move);
    glm_vec3_add(state->camera.position, move, state->camera.position);
  } else if (keyboard_state[SDL_SCANCODE_S]) {
    glm_vec3_scale(state->camera.direction, camera_speed, move);
    glm_vec3_add(state->camera.position, move, state->camera.position);
  }

  if (keyboard_state[SDL_SCANCODE_A]) {
    glm_vec3_cross(state->camera.direction, state->camera.up, move);
    glm_vec3_normalize(move);
    glm_vec3_scale(move, camera_speed, move);
    glm_vec3_add(state->camera.position, move, state->camera.position);
  } else if (keyboard_state[SDL_SCANCODE_D]) {
    glm_vec3_cross(state->camera.direction, state->camera.up, move);
    glm_vec3_normalize(move);
    glm_vec3_scale(move, camera_speed, move);
    glm_vec3_sub(state->camera.position, move, state->camera.position);
  }
}

SDL_AppResult plugin_loop(void *appstate) {
  (void) appstate;

  const double now = ((double)SDL_GetTicks()) / 1000.0;
  state->delta_time = now - state->last_frame_time;
  state->last_frame_time = now;
  
  handle_keyboard();
  update_camera(&state->camera);

  glClearColor(RGBAF(0x330000FF));
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


  mat4 projection;
  glm_perspective(glm_rad(45.0f), state->aspect, 0.1f, 100.0f, projection);

  {
    mat4 model; glm_mat4_identity(model);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glUseProgram(state->axes.program);
    glUniformMatrix4fv(state->axes.u_model, 1, GL_FALSE, (float *)model);
    glUniformMatrix4fv(state->axes.u_view, 1, GL_FALSE, (float *)state->camera.view);
    glUniformMatrix4fv(state->axes.u_projection, 1, GL_FALSE, (float *)projection);
    draw_axes(state->axes.vao);
  }

  {
    mat4 model; glm_mat4_identity(model);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glUseProgram(state->grid.program);
    glUniformMatrix4fv(state->grid.u_model, 1, GL_FALSE, (float *)model);
    glUniformMatrix4fv(state->grid.u_view, 1, GL_FALSE, (float *)state->camera.view);
    glUniformMatrix4fv(state->grid.u_projection, 1, GL_FALSE, (float *)projection);
    draw_grid(state->grid.vao);
  }
  
  {
    vec4 vec = {1.0f, 0.0f, 0.0f, 1.0f};
    mat4 model; glm_mat4_identity(model);
    glm_scale(model, (vec3){0.5f, 0.5f, 0.5f});
    glm_rotate_x(model, glm_rad(55.0f), model);
    glm_rotate_y(model, glm_rad(now * 90.0f), model);
    glm_mat4_mulv(model, vec, vec);
  
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glUseProgram(state->cube.program);
    glUniform1f(state->cube.u_time, now);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, state->cube.texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, state->cube.texture2);
    glUniformMatrix4fv(state->cube.u_model, 1, GL_FALSE, (float *)model);
    glUniformMatrix4fv(state->cube.u_view, 1, GL_FALSE, (float *)state->camera.view);
    glUniformMatrix4fv(state->cube.u_projection, 1, GL_FALSE, (float *)projection);
    draw_cube(state->cube.vao);
  }

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
