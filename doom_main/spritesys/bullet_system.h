#ifndef BULLET_SYSTEM_H
#define BULLET_SYSTEM_H
#include <glad/glad.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void init_bullet_system(void);
void fire_bullet(void);
void update_bullet_system(float dt);
void draw_bullets(void);

#endif