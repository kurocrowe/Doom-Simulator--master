#include "shader.h"
#define _CRT_SECURE_NO_WARNINGS
GLuint create_shader(const char *vertex_path, const char *fragment_path) {
  GLint success;
  char  infoLog[512];

  // Compile vertex shader
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_path, NULL);
  glCompileShader(vertex_shader);

  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex_shader, sizeof(infoLog), NULL, infoLog);
    printf("Vertex Shader Compilation Failed:\n%s\n", infoLog);
  }

  // Compile fragment shader
  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_path, NULL);
  glCompileShader(fragment_shader);

  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment_shader, sizeof(infoLog), NULL, infoLog);
    printf("Fragment Shader Compilation Failed:\n%s\n", infoLog);
  }

  // Link shaders into program
  GLuint program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, sizeof(infoLog), NULL, infoLog);
    printf("Shader Program Linking Failed:\n%s\n", infoLog);
  }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  return program;
}