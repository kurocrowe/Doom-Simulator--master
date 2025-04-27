#include "bullet_system.h"
#include "C:\Users\fores\OneDrive\桌面\doom_main_final\doom_main_dab\doom_main_dab\doom_main\doom_main\camera.h"
#include "enemy_system.h"
#define MAX_BULLETS 128
#include "shader.h"
#define M_PI 3.14159265358979323846264338327950288
typedef struct {
  vec3_t position;
  vec3_t velocity;
  float  lifetime;
  bool   active;
} bullet_t;

static bullet_t bullets[MAX_BULLETS];
static GLuint   bullet_texture;
static GLuint   bullet_shader;
static GLuint   vao, vbo;

extern camera_t camera;

// --- Shader Code Embedded ---
const char *bullet_vertex_shader_src =
    "#version 330 core\n"
    "layout(location = 0) in vec3 aPos;\n"
    "layout(location = 1) in vec2 aTexCoord;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "out vec2 TexCoord;\n"
    "void main() {\n"
    "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "    TexCoord = aTexCoord;\n"
    "}\n";

const char *bullet_fragment_shader_src =
    "#version 330 core\n"
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D uTexture;\n"
    "void main()\n"
    "{\n"
    "    FragColor = texture(uTexture, TexCoord);\n"
    "}\n";

// --- Init Bullet System ---
void init_bullet_system() {
  bullet_shader = create_shader(bullet_vertex_shader_src,
                                            bullet_fragment_shader_src);

  float vertices[] = {
      -0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f,  0.0f, 0.0f, 1.0f, 0.0f,
      0.1f,  0.2f, 0.0f, 1.0f, 1.0f, -0.1f, 0.2f, 0.0f, 0.0f, 1.0f,
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
  bullet_texture = load_texture("spritesys/bullet.png");
}

// --- Fire a Bullet ---
void fire_bullet() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) {
      bullets[i].position = camera.position;
      bullets[i].velocity = vec3_scale(camera.forward, 10.0f);
      bullets[i].lifetime = 3.0f; // Bullet lives for 3 seconds
      bullets[i].active   = true;
      break;
    }
  }
}

// --- Update Bullets ---
void update_bullet_system(float dt) {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      bullets[i].position =
          vec3_add(bullets[i].position, vec3_scale(bullets[i].velocity, dt));
      bullets[i].lifetime -= dt;
      if (bullets[i].lifetime <= 0.0f) { bullets[i].active = false; }
    }
  }
}

// --- Draw Bullets ---
void draw_bullets() {
  glUseProgram(bullet_shader);

  mat4_t view = mat4_look_at(
      camera.position, vec3_add(camera.position, camera.forward), camera.up);
  mat4_t proj = mat4_perspective(M_PI / 3.0f, 800.0f / 600.0f, 0.1f, 1000.0f);

  glUniformMatrix4fv(glGetUniformLocation(bullet_shader, "view"), 1, GL_FALSE,
                     &view.m[0][0]);
  glUniformMatrix4fv(glGetUniformLocation(bullet_shader, "projection"), 1,
                     GL_FALSE, &proj.m[0][0]);

  glBindVertexArray(vao);
  glBindTexture(GL_TEXTURE_2D, bullet_texture);
  glUniform1i(glGetUniformLocation(bullet_shader, "uTexture"), 0);

  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) continue;

    mat4_t model = mat4_translate(bullets[i].position);
    model        = mat4_mul(model, mat4_scale((vec3_t){0.5f, 0.5f, 0.5f}));

    glUniformMatrix4fv(glGetUniformLocation(bullet_shader, "model"), 1,
                       GL_FALSE, &model.m[0][0]);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }

  glBindVertexArray(0);
}