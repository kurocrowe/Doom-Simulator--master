#include "enemy_system.h"
#include "shader.h"
#define SCREEN_W 800
#define SCREEN_H 600
#include <glad/glad.h>
#include <stdio.h>
#include "C:\Users\fores\OneDrive\桌面\doom_main_final\doom_main_dab\doom_main_dab\doom_main\doom_main\camera.h"
#include "C:\Users\fores\OneDrive\桌面\doom_main_final\doom_main_dab\doom_main_dab\doom_main\doom_main\matrix.h"
#include <corecrt_math_defines.h>
#include <math.h>
#include "C:\Users\fores\OneDrive\桌面\doom_main_final\doom_main_dab\doom_main_dab\doom_main\doom_main\map.h"
#define MAX_ENEMIES 16
camera_t camera;
int     player_health;
float   damage_flash_timer;
static enemy_t enemies[MAX_ENEMIES];
static GLuint enemy_shader;
static GLuint vao, vbo;
static GLuint enemy_textures[8];
static int    num_enemies = 0;
float          zBuffer[SCREEN_W];
// === Embedded Shader Source Code ===

static float enemy_speed = 1.5f; // units per second

// --- Shader Source Code (embedded) ---
//const char *enemy_vertex_shader_src =
//    "#version 330 core\n"
//    "layout(location = 0) in vec3 aPos;\n"
//    "layout(location = 1) in vec2 aTexCoord;\n"
//    "uniform mat4 model;\n"
//    "uniform mat4 view;\n"
//    "uniform mat4 projection;\n"
//    "out vec2 TexCoord;\n"
//    "void main() {\n"
//    "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
//    "    TexCoord = aTexCoord;\n"
//    "}\n";
//
//const char *enemy_fragment_shader_src =
//    "#version 330 core\n"
//    "in vec2 TexCoord;\n"
//    "out vec4 FragColor;\n"
//    "uniform sampler2D uTexture;\n"
//    "void main() {\n"
//    "    FragColor = texture(uTexture, TexCoord);\n"
//    "}\n";
const char *enemy_vertex_shader_src =
    "#version 330 core\n"
    "layout(location = 0) in vec3 aPos;\n"
    "layout(location = 1) in vec2 aTexCoord;\n"
    "out vec2 vTexCoord;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main() {\n"
    "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "    vTexCoord = aTexCoord;\n"
    "}\n";
const char *enemy_fragment_shader_src =
    "#version 330 core\n"
    "in vec2 vTexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D uTexture;\n"
    "void main() {\n"
    "    FragColor = texture(uTexture, vTexCoord);\n"
    "}\n";




// Monster types in Doom
const int monster_type_ids[] = {3001, 3002, 3003, 3004, 3005, 58};
#define NUM_MONSTER_TYPES                                                      \
  (sizeof(monster_type_ids) / sizeof(monster_type_ids[0]))

static bool is_monster(int type) {
  for (int i = 0; i < NUM_MONSTER_TYPES; i++) {
    if (monster_type_ids[i] == type) { return true; }
  }
  return false;
}
void spawn_random_enemies(int count) {
  for (int i = 0; i < count; i++) {
    float angle = ((float)rand() / RAND_MAX) * 2.0f * M_PI; // Random 0 to 2π
    float distance =
        5.0f + ((float)rand() / RAND_MAX) * 10.0f; // Random 5–15 units

    float x = camera.position.x + cosf(angle) * distance;
    float y = camera.position.z + sinf(angle) * distance;

    vec2_t pos = {x, y};

    if (map_get_sector(pos)) { // Check if inside valid map area
      enemies[num_enemies++] = (enemy_t){.position     = pos,
                                         .health       = 100,
                                         .attack_timer = 0.0f,
                                         .is_attacking = false};
      if (num_enemies >= MAX_ENEMIES) break;
    }
  }
}
 GLuint enemy_tex;
// --- Initialize Enemies ---
void init_enemy_system() {
  enemy_shader =
      create_shader(enemy_vertex_shader_src, enemy_fragment_shader_src);
  enemy_tex = load_texture("spritesys/monster_idle0.png"); // Your enemy sprite
  // Simple quad vertex
  float vertices[] = {
      //   x      y    z     u    v
      -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, // Bottom-left
      0.5f,  0.0f, 0.0f, 1.0f, 0.0f, // Bottom-right
      0.5f,  1.0f, 0.0f, 1.0f, 1.0f, // Top-right
      -0.5f, 1.0f, 0.0f, 0.0f, 1.0f  // Top-left
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

  num_enemies = 0; // Clear enemies

  // --- Spawn enemies ---
  for (int i = 0; i < map.num_things; i++) {
    thing_t *thing = &map.things[i];

    // TEMPORARY: Spawn at ALL Things
    vec2_t pos = {thing->position.x, thing->position.y};

    enemies[num_enemies++] = (enemy_t){.position     = pos,
                                       .health       = 100,
                                       .attack_timer = 0.0f,
                                       .is_attacking = false,
                                                };

    if (num_enemies >= MAX_ENEMIES) break; // Don't overflow
  }

  printf("[INFO] Spawned %d enemies from map\n", num_enemies);
}


// --- Update enemy logic each frame ---
void update_enemy_system(float dt) {
  for (int i = 0; i < num_enemies; i++) {
    enemy_t *e = &enemies[i];
    if (e->health <= 0) continue; // Skip dead enemies

    // Calculate vector from enemy to player
    float dx       = camera.position.x - e->position.x;
    float dy       = camera.position.z - e->position.y;
    float distance = sqrtf(dx * dx + dy * dy);

    if (distance > 0.01f) {
      // Normalize direction
      float dir_x = dx / distance;
      float dir_y = dy / distance;

      // Enemy moves toward the player
      vec2_t next_pos = {e->position.x + dir_x * enemy_speed * dt,
                         e->position.y + dir_y * enemy_speed * dt};

      // Only update if walkable (no wall blocking)
      if (is_walkable(next_pos)) { e->position = next_pos; }
    }

    // --- Check if close enough to attack
    if (distance < 1.5f) { // ~1.5 units = close combat
      e->is_attacking = true;
      e->attack_timer += dt;

      if (e->attack_timer >= 1.0f) { // Attack once per second
        player_health -= 10;         // 🔥 Deal damage to player
        if (player_health < 0) player_health = 0;
        damage_flash_timer = 0.3f; // Flash screen red
        e->attack_timer    = 0.0f; // Reset attack cooldown
      }
    } else {
      e->is_attacking = false;
      e->attack_timer = 0.0f;
    }
  }
}

// --- Draw enemies (cuboid) ---
void draw_enemies() {
  glUseProgram(enemy_shader);

  mat4_t view = mat4_look_at(
      camera.position, vec3_add(camera.position, camera.forward), camera.up);
  mat4_t proj = mat4_perspective(M_PI / 3.0f, (float)SCREEN_W / (float)SCREEN_H,
                                 0.1f, 1000.0f);

  glUniformMatrix4fv(glGetUniformLocation(enemy_shader, "view"), 1, GL_FALSE,
                     &view.m[0][0]);
  glUniformMatrix4fv(glGetUniformLocation(enemy_shader, "projection"), 1,
                     GL_FALSE, &proj.m[0][0]);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, enemy_tex); // ✅ bind once
  glUniform1i(glGetUniformLocation(enemy_shader, "uTexture"),
              0); // Set texture unit

  glBindVertexArray(vao);

  for (int i = 0; i < num_enemies; i++) {
    enemy_t *e = &enemies[i];
    if (e->health <= 0) continue;

    // Build model matrix
    mat4_t model = mat4_identity();
    model        = mat4_translate((vec3_t){e->position.x, 0.0f, e->position.y});
    model = mat4_scale_xyz(model, (vec3_t){50.0f, 50.0f, 50.0f}); // ✅ 50x size

    glUniformMatrix4fv(glGetUniformLocation(enemy_shader, "model"), 1, GL_FALSE,
                       &model.m[0][0]);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }

  glBindVertexArray(0);
}


enemy_t *enemy_get(int index) {
  if (index < 0 || index >= num_enemies) return NULL;
  return &enemies[index];
}

int enemy_get_count() { return num_enemies; }

void enemy_damage(enemy_t *enemy, int amount) {
  if (!enemy) return;
  enemy->health -= amount;
  if (enemy->health <= 0) { enemy->health = 0; }
}