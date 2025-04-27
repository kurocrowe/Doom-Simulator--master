// enemy_system.h
#ifndef ENEMY_SYSTEM_H
#define ENEMY_SYSTEM_H
#include <math.h>
#include "C:\Users\fores\OneDrive\桌面\doom_main_final\doom_main_dab\doom_main_dab\doom_main\doom_main\vector.h"
#include <glad/glad.h>
#include "C:\Users\fores\OneDrive\桌面\doom_main_final\doom_main_dab\doom_main_dab\doom_main\doom_main\engine.h"
typedef enum {
  ENEMY_STATE_IDLE,
  ENEMY_STATE_ATTACK,
  ENEMY_STATE_HURT,
  ENEMY_STATE_DEAD
} enemy_state_t;

typedef struct {
  vec2_t        position;
  int           health;
  GLuint        texture;
  float         attack_timer;
  bool          is_attacking;
  enemy_state_t state;
  float         state_timer;
  GLuint        tex_idle;
  GLuint        tex_attack[2];
  GLuint        tex_pain;
  GLuint        tex_death[3];
  int           death_frame;
} enemy_t;

void init_enemy_system();
void update_enemy_system(float dt);
void draw_enemies();

enemy_t *enemy_get(int index);
int      enemy_get_count();
void     enemy_damage(enemy_t *enemy, int amount);

#endif // ENEMY_SYSTEM_H
