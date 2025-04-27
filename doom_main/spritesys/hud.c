#include "hud.h"
#include "shader.h"
#include <glad/glad.h>
#include <stdio.h>
#include "C:\Users\fores\OneDrive\桌面\doom_main_final\doom_main_dab\doom_main_dab\doom_main\doom_main\vector.h"
#include "C:\Users\fores\OneDrive\桌面\doom_main_final\doom_main_dab\doom_main_dab\doom_main\doom_main\matrix.h"
int   player_health;
float damage_flash_timer;

static GLuint hud_shader;
static GLuint vao, vbo;

// --- Embedded Shader Source Code for HUD ---
const char *hud_vertex_shader_src =
    "#version 330 core\n"
    "layout(location = 0) in vec2 aPos;\n"
    "layout(location = 1) in vec2 aTexCoord;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "out vec2 TexCoord;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = projection * view * model * vec4(aPos, 0.0, 1.0);\n"
    "    TexCoord = aTexCoord;\n"
    "}\n";

const char *hud_fragment_shader_src =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec4 uColor;\n"
    "void main()\n"
    "{\n"
    "    FragColor = uColor;\n"
    "}\n";

// --- Initialize HUD System ---
void init_hud() {
  hud_shader =
      create_shader(hud_vertex_shader_src, hud_fragment_shader_src);

  float vertices[] = {
      // positions     // texcoords
      0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
      1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
  };

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

// --- Helper to draw rectangle ---
static void draw_rect(float x, float y, float w, float h, vec4_t color) {
  glUseProgram(hud_shader);

  glDisable(GL_DEPTH_TEST);
  glBindVertexArray(vao);

  mat4_t model = mat4_scale((vec3_t){w, h, 1.0f});
  model        = mat4_mul(mat4_translate((vec3_t){x, y, 0.0f}), model);

  mat4_t view = mat4_identity();
  mat4_t proj = mat4_ortho(0, 1, 0, 1, -1, 1); // Orthographic 2D

  glUniformMatrix4fv(glGetUniformLocation(hud_shader, "model"), 1, GL_FALSE,
                     &model.m[0][0]);
  glUniformMatrix4fv(glGetUniformLocation(hud_shader, "view"), 1, GL_FALSE,
                     &view.m[0][0]);
  glUniformMatrix4fv(glGetUniformLocation(hud_shader, "projection"), 1,
                     GL_FALSE, &proj.m[0][0]);
  glUniform4f(glGetUniformLocation(hud_shader, "uColor"), color.x, color.y,
              color.z, color.w);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glBindVertexArray(0);
  glEnable(GL_DEPTH_TEST);
}

// --- Draw HUD elements (health bar, damage flash) ---
void draw_hud() {
  // Health Bar
  float health_percent = (float)player_health / 100.0f;
  if (health_percent < 0.0f) health_percent = 0.0f;

  // Health bar background (gray)
  draw_rect(0.05f, 0.9f, 0.4f, 0.05f, (vec4_t){0.2f, 0.2f, 0.2f, 1.0f});
  // Health bar fill (red)
  draw_rect(0.05f, 0.9f, 0.4f * health_percent, 0.05f,
            (vec4_t){1.0f, 0.0f, 0.0f, 1.0f});

  // Flash red when damaged
  if (damage_flash_timer > 0.0f) {
    draw_rect(0.0f, 0.0f, 1.0f, 1.0f,
              (vec4_t){1.0f, 0.0f, 0.0f, damage_flash_timer});
  }
}
void update_hud(float dt) {
  // Reduce damage flash timer over time
  if (damage_flash_timer > 0.0f) {
    damage_flash_timer -= dt;
    if (damage_flash_timer < 0.0f) { damage_flash_timer = 0.0f; }
  }
}