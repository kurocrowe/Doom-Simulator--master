#include "weapon_system.h"
#include "shader.h"
#include "C:\Users\fores\OneDrive\桌面\doom_main_final\doom_main_dab\doom_main_dab\doom_main\doom_main\engine.h"
#include <glad/glad.h>
#include <stdio.h>
#include "C:\Users\fores\OneDrive\桌面\doom_main_final\doom_main_dab\doom_main_dab\doom_main\doom_main\matrix.h"
static GLuint weapon_shader;
static GLuint vao, vbo;
static GLuint weapon_textures[8];
static int    num_weapons    = 0;
static int    current_weapon = 0;
static float  weapon_bob_timer  = 0.0f;
static float  weapon_bob_offset = 0.0f;
// === Embedded Shader Source Code ===

const char *weapon_vertex_shader_src =
    "#version 330 core\n"
    "layout(location = 0) in vec3 aPos;\n"
    "layout(location = 1) in vec2 aTexCoord;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "out vec2 TexCoord;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "    TexCoord = aTexCoord;\n"
    "}\n";

const char *weapon_fragment_shader_src =
    "#version 330 core\n"
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D uTexture;\n"
    "void main()\n"
    "{\n"
    "    FragColor = texture(uTexture, TexCoord);\n"
    "}\n";

// === Weapon System Code ===

void init_weapon_system() {
  weapon_shader = create_shader(weapon_vertex_shader_src,
                                            weapon_fragment_shader_src);

  float vertices[] = {
      -0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3f,  0.0f, 0.0f, 1.0f, 0.0f,
      0.3f,  0.4f, 0.0f, 1.0f, 1.0f, -0.3f, 0.4f, 0.0f, 0.0f, 1.0f,
  };

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
  stbi_set_flip_vertically_on_load(true);
  weapon_textures[num_weapons++] = load_texture("spritesys/shoot.jpg");
  weapon_textures[num_weapons++] = load_texture("spritesys/weapon.png");
}

void update_weapon_system(float dt) {
  weapon_bob_timer += dt * 3.0f;
  weapon_bob_offset = sinf(weapon_bob_timer) * 0.02f;
}

void draw_weapon() {

  GLuint tex = weapon_textures[current_weapon];
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glUseProgram(weapon_shader);

  glDisable(GL_DEPTH_TEST);
  glBindVertexArray(vao);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);
  glUniform1i(glGetUniformLocation(weapon_shader, "uTexture"), 0);

  mat4_t model = mat4_translate((vec3_t){0.0f, -1.0f, 0.0f});
  mat4_t view  = mat4_identity();
  mat4_t proj  = mat4_ortho(-1, 1, -1, 1, -1, 1);

  glUniformMatrix4fv(glGetUniformLocation(weapon_shader, "model"), 1, GL_FALSE,
                     &model.m[0][0]);
  glUniformMatrix4fv(glGetUniformLocation(weapon_shader, "view"), 1, GL_FALSE,
                     &view.m[0][0]);
  glUniformMatrix4fv(glGetUniformLocation(weapon_shader, "projection"), 1,
                     GL_FALSE, &proj.m[0][0]);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glBindVertexArray(0);
  glDisable(GL_BLEND);
}
void switch_weapon(int weapon_id) {
  if (weapon_id >= 0 && weapon_id < num_weapons) { current_weapon = weapon_id; }
}