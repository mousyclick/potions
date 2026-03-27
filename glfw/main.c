#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define WIDTH 900
#define HEIGHT 900

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint load_container_texture() {
  GLuint id;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
  int width, height, channels;
  unsigned char *data = stbi_load("container.jpg", &width, &height, &channels, 0);
  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(data);
  return id;
}

GLuint load_face_texture() {
  GLuint id;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  stbi_set_flip_vertically_on_load(true);  
  int width, height, channels;
  unsigned char *data = stbi_load("awesomeface.png", &width, &height, &channels, 0);
  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
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

void error_callback(int error, const char* description) {
  fprintf(stderr, "ERROR: %d %s\n", error, description);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  (void)window;
  glViewport(0, 0, width, height);
}

GLuint create_triangle() {
  float verts[] = {
    -0.75f, -0.75f, 0.0f,
    0.75f, -0.75f, 0.0f,
    0.0f, 0.75f, 0.0f
  };
  
  GLuint VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  
  GLuint VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
  return VAO;
}

GLuint create_colored_triangle() {
  float verts[] = {
    -0.6f, -0.6f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.6f, -0.6f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.6f, 0.0f, 0.0f, 0.0f, 1.0f,
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

GLuint create_square() {
  float verts[] = {
    0.5f, 0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
    -0.5f, 0.5f, 0.0f
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
  
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
  return VAO;
}

GLuint create_textured_square() {
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

void draw_triangle(GLuint vao) {
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

void draw_square(GLuint vao) {
  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void print_gl_info() {
  GLint gl_major, gl_minor;
  glGetIntegerv(GL_MAJOR_VERSION, &gl_major);
  glGetIntegerv(GL_MINOR_VERSION, &gl_minor);
    
  printf("INFO: OpenGL version %d.%d\n", gl_major, gl_minor);

  GLint vattr;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &vattr);
  printf("INFO: supports %d vertex attributes\n", vattr);
}

int main(void) {
  glfwSetErrorCallback(error_callback);
  
  if (!glfwInit()) {
    fprintf(stderr, "ERROR: couldn't initialize GLFW\n");
    exit(1);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Hello GLFW", NULL, NULL);
  if (!window) {
    fprintf(stderr, "Failed to create window\n");
    glfwTerminate();
    exit(1);
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  

  GLenum err = glewInit();
  if (GLEW_OK != err) {
    fprintf(stderr, "ERROR: couldn't initialize GLEW: %s\n", glewGetErrorString(err));
    return 1;
  }

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  print_gl_info();

  GLuint container_texture = load_container_texture();
  GLuint face_texture = load_face_texture();
 
  GLuint triangle = create_triangle();
  GLuint colored_triangle = create_colored_triangle();
  GLuint square = create_square();
  GLuint textured_square = create_textured_square();

  GLuint vert_shader = compile_shader("shader.vert.glsl", GL_VERTEX_SHADER);
  GLuint frag_shader = compile_shader("shader.frag.glsl", GL_FRAGMENT_SHADER);
  GLuint shader_program = link_program(vert_shader, frag_shader);
  glDeleteShader(frag_shader);
  glDeleteShader(vert_shader);

  GLuint colored_vert_shader = compile_shader("shader.colored.vert.glsl", GL_VERTEX_SHADER);
  GLuint colored_frag_shader = compile_shader("shader.colored.frag.glsl", GL_FRAGMENT_SHADER);
  GLuint colored_shader_program = link_program(colored_vert_shader, colored_frag_shader);
  glDeleteShader(colored_frag_shader);
  glDeleteShader(colored_vert_shader);

  GLuint textured_vert_shader = compile_shader("shader.textured.vert.glsl", GL_VERTEX_SHADER);
  GLuint textured_frag_shader = compile_shader("shader.textured.frag.glsl", GL_FRAGMENT_SHADER);
  GLuint textured_shader_program = link_program(textured_vert_shader, textured_frag_shader);
  glDeleteShader(textured_frag_shader);
  glDeleteShader(textured_vert_shader);
  
  GLuint utime_loc = glGetUniformLocation(shader_program, "u_time");
  GLuint colored_utime_loc = glGetUniformLocation(colored_shader_program, "u_time");
  GLuint textured_utime_loc = glGetUniformLocation(textured_shader_program, "u_time");
  glUseProgram(textured_shader_program);
  GLuint u_container_texture = glGetUniformLocation(textured_shader_program, "u_container_texture");
  glUniform1i(u_container_texture, 0);
  GLuint u_face_texture = glGetUniformLocation(textured_shader_program, "u_face_texture");
  glUniform1i(u_face_texture, 1);

#if 0
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#else
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
  
  while (!glfwWindowShouldClose(window)) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, true);
    }
    
    glClearColor(0.1f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_program);
    glUniform1f(utime_loc, glfwGetTime());
    draw_triangle(triangle);

    glUniform1f(utime_loc, glfwGetTime() + 3.14f);
    draw_square(square);

    glUseProgram(colored_shader_program);
    glUniform1f(colored_utime_loc, glfwGetTime());
    draw_triangle(colored_triangle);
    
    glUseProgram(textured_shader_program);
    glUniform1f(textured_utime_loc, glfwGetTime());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, container_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, face_texture);
    draw_square(textured_square);
    
    glfwPollEvents();
    glfwSwapBuffers(window);
  }
  glfwTerminate();

  return 0;
}
