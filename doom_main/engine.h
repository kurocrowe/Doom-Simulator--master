#ifndef _ENGINE_H
#define _ENGINE_H
#include "stb_image.h"
#include "anim.h"
#include "camera.h"
#include "flat_texture.h"
#include "gl_map.h"
#include "input.h"
#include "map.h"
#include "matrix.h"
#include "mesh.h"
#include "meshgen.h"
#include "palette.h"
#include "renderer.h"
#include "state.h"
#include "util.h"
#include "vector.h"
#include "wad.h"
#include "wall_texture.h"
#include "util.h"

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct {
  GLuint id;
  int    width;
  int    height;
} texture_t;

typedef struct {
  float x, y, w, h;
} rect_t;


void engine_init(wad_t *wad, const char *mapname);
void engine_update(float dt);
void engine_render();
#endif // !_ENGINE_H
void init_menus(void);
GLuint load_texture(const char *filename);
void   draw_damage_flash();
void   generate_quad3(vec2_t center, vec2_t size, int texture_id);