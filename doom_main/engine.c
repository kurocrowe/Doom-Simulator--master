
#include "engine.h"
#include "stb.h"
#include "spritesys/bullet_system.h"
#include "spritesys/hud.h"
#include "spritesys/enemy_system.h"
#include "spritesys/weapon_system.h"
float       damage_flash_timer = 0.0f;
bool      is_walkable(vec2_t new_pos, float eye_y);
int         player_health;

static texture_t load_texture2(const char *path);
#define FRAMEBUFFER_WIDTH  800
#define FRAMEBUFFER_HEIGHT 600
typedef struct {
  uint8_t r, g, b, a;
} color_t;
static void put_pixel(int x, int y, color_t color);
static void draw_colored_quad1
(rect_t rect, color_t color);

    typedef struct {
  vec3_t  position;
  vec2_t  pos;
  vec2_t  uv;
  color_t color;
} vertex1_t;
static void draw_colored_triangles(vertex1_t *verts, int count);

static void draw_textured_triangles2(vertex1_t *vertices, int count,
                                     int texture_id);
color_t     framebuffer[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];
#define _CRT_SECURE_NO_WARNINGS
#define FOV               (M_PI / 3.f)
#define PLAYER_SPEED      (500.f)
#define MOUSE_SENSITIVITY (.05f)
#define M_PI              3.14159265358979323846264338327950288
#define M_PI_2            1.5707963267948966
static void render_node(draw_node_t *node);
static sector_t *find_sector_for_position(vec2_t pos);
size_t           num_flats, num_wall_textures, num_palettes;
wall_tex_info_t *wall_textures_info;
vec2_t          *wall_max_coords;
vertex1_t        vertex;
map_t    map;
gl_map_t gl_map;
float    player_height;
float    max_sector_height;
int      sky_flat;
bool             noclip_enabled = false;
bool             was_n_pressed  = false;
draw_node_t   *root_draw_node;
stencil_list_t stencil_list;
mesh_t         quad_mesh;

static float camera_y_velocity = 0.0f;
static bool  is_jumping        = false;
const float  gravity           = -500.0f;
const float  jump_strength     = 100.0f;



typedef enum {
  GAME_STATE_START_MENU,
  GAME_STATE_PLAYING,
  GAME_STATE_PAUSED
} game_state_t;



typedef struct {
  rect_t     bounds;
  texture_t  texture;
  texture_t  hover_tex;
  bool       is_hovered;
  bool       was_clicked;
} button2_t;
#define SCREEN_W 800
#define SCREEN_H 600

float zBuffer[SCREEN_W];
#define MAX_BUTTONS 8

typedef struct {
  button2_t buttons[MAX_BUTTONS];
  int      count;
} menu_t;
typedef struct {
  float x, y; // position
  float u, v; // texture coords
} vertex3_t;
camera_t      camera;
static GLuint vao, vbo;
static GLuint shader_program;

// Vertex and fragment shader sources
const char *vertex_shader_src =
    "#version 330 core\n"
    "layout(location = 0) in vec2 a_position;\n"
    "layout(location = 1) in vec2 a_texcoord;\n"
    "out vec2 v_texcoord;\n"
    "void main() {\n"
    "    gl_Position = vec4(a_position, 0.0, 1.0);\n"
    "    v_texcoord = a_texcoord;\n"
    "}\n";



const char *fragment_shader_src =
    "#version 330 core\n"
    "in vec2 v_texcoord;\n"
    "out vec4 frag_color;\n"
    "uniform sampler2D u_texture;\n"
    "void main() {\n"
    "    frag_color = texture(u_texture, v_texcoord);\n"
    "}\n";
map_t map;

// Compile and link shaders

 static void init_shader_program() {
  GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader_src);
  GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_src);

  shader_program = glCreateProgram();
  glAttachShader(shader_program, vs);
  glAttachShader(shader_program, fs);
  glLinkProgram(shader_program);

  glDeleteShader(vs);
  glDeleteShader(fs);
}

// Initialize quad geometry
static void init_quad(float x, float y, float w, float h) {
  vertex3_t quad_vertices[6] = {
      {x,     y,     0.0f, 0.0f},
      {x + w, y,     1.0f, 0.0f},
      {x + w, y + h, 1.0f, 1.0f},

      {x,     y,     0.0f, 0.0f},
      {x + w, y + h, 1.0f, 1.0f},
      {x,     y + h, 0.0f, 1.0f}
  };

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex3_t), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex3_t),
                        (void *)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

// Draw the quad
static void draw_quad(float r, float g, float b, float a) {
  glUseProgram(shader_program);

  GLint color_loc = glGetUniformLocation(shader_program, "u_color");
  glUniform4f(color_loc, r, g, b, a);

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}

static game_state_t current_game_state = GAME_STATE_START_MENU;
static menu_t       start_menu         = {0};
static menu_t       pause_menu         = {0};

static bool point_in_rect(vec2_t point, rect_t rect) {
  return point.x >= rect.x && point.x <= rect.x + rect.w && point.y >= rect.y &&
         point.y <= rect.y + rect.h;
}

void update_menu(menu_t *menu, vec2_t mouse_pos, bool mouse_pressed) {
  for (int i = 0; i < menu->count; i++) {
    button2_t *btn    = &menu->buttons[i];
    btn->is_hovered  = point_in_rect(mouse_pos, btn->bounds);
    btn->was_clicked = btn->is_hovered && mouse_pressed;
  }
}


void init_menus(void) {
  start_menu.count      = 1;
  start_menu.buttons[0] = (button2_t){
      .bounds    = {300, 200, 200, 64},
      .texture   = load_texture2("start.jpg"),
      .hover_tex = load_texture2("starthover.jpg")
  };

  pause_menu.count      = 1;
  pause_menu.buttons[0] = (button2_t){
      .bounds    = {300, 300, 200, 64},
      .texture   = load_texture2("pause.jpg"),
      .hover_tex = load_texture2("pausehover.jpg")
  };
}
size_t         num_tex_anim_defs;
tex_anim_def_t tex_anim_defs[] = {
    {"NUKAGE3", "NUKAGE1"},
    {"FWATER4", "FWATER1"},
    {"SWATER4", "SWATER1"},
    {"LAVA4",   "LAVA1"  },
    {"BLOOD3",  "BLOOD1" },
};


static vec2_t   last_mouse;

void engine_init(wad_t *wad, const char *mapname) {

  vec2_t size       = renderer_get_size();
  mat4_t projection = mat4_perspective(FOV, size.x / size.y, .1f, 10000.f);
  renderer_set_projection(projection);

  char *gl_mapname = malloc(strlen(mapname) + 4);
  gl_mapname[0]    = 'G';
  gl_mapname[1]    = 'L';
  gl_mapname[2]    = '_';
  gl_mapname[3]    = 0;
  strcat_s(gl_mapname, sizeof(gl_mapname), mapname);
  if (wad_read_gl_map(gl_mapname, &gl_map, wad) != 0) {
    fprintf(stderr, "Failed to read GL info for map (%s) from WAD file\n",
            mapname);
    return;
  }

  palette_t *palettes    = wad_read_playpal(&num_palettes, wad);
  GLuint palette_texture = palettes_generate_texture(palettes, num_palettes);

  sky_flat = wad_find_lump("F_SKY1", wad) - wad_find_lump("F_START", wad) - 1;

  num_tex_anim_defs = sizeof tex_anim_defs / sizeof tex_anim_defs[0];
  for (int i = 0; i < num_tex_anim_defs; i++) {
    tex_anim_defs[i].start = tex_anim_defs[i].end = -1;
  }

  flat_tex_t *flats         = wad_read_flats(&num_flats, wad);
  GLuint flat_texture_array = generate_flat_texture_array(flats, num_flats);
  for (int i = 0; i < num_flats; i++) {
    for (int j = 0; j < num_tex_anim_defs; j++) {
      if (strcmp_nocase(flats[i].name, tex_anim_defs[j].start_name) == 0) {
        tex_anim_defs[j].start = i;
      }

      if (strcmp_nocase(flats[i].name, tex_anim_defs[j].end_name) == 0) {
        tex_anim_defs[j].end = i;
      }
    }
  }
  free(flats);

  wall_tex_t *textures = wad_read_textures(&num_wall_textures, "TEXTURE1", wad);
  wall_textures_info   = malloc(sizeof(wall_tex_info_t) * num_wall_textures);
  wall_max_coords      = malloc(sizeof(vec2_t) * num_wall_textures);
  for (int i = 0; i < num_wall_textures; i++) {
    if (strcmp_nocase(textures[i].name, "SKY1") == 0) {
      renderer_set_sky_texture(generate_texture_cubemap(&textures[i]));
    }

    wall_textures_info[i] =
        (wall_tex_info_t){textures[i].width, textures[i].height};
  }
  GLuint wall_texture_array =
      generate_wall_texture_array(textures, num_wall_textures, wall_max_coords);
  wad_free_wall_textures(textures, num_wall_textures);

  if (wad_read_map(mapname, &map, wad, textures, num_wall_textures) != 0) {
    fprintf(stderr, "Failed to read map (%s) from WAD file\n", mapname);
    return;
  }

  for (int i = 0; i < map.num_things; i++) {
    thing_t *thing = &map.things[i];

    if (thing->type == THING_P1_START) {
      thing_info_t *info = NULL;

      for (int i = 0; i < map_num_thing_infos; i++) {
        if (thing->type == map_thing_info[i].type) {
          info = &map_thing_info[i];
          break;
        }
      }

      if (info == NULL) { continue; }

      player_height = info->height;

      camera = (camera_t){
          .position = {thing->position.x, player_height, thing->position.y},
          .yaw      = thing->angle,
          .pitch    = 0.f,
      };
    }
  }

  stencil_list = (stencil_list_t){NULL, NULL};
  generate_meshes();

  renderer_set_flat_texture(flat_texture_array);
  renderer_set_wall_texture(wall_texture_array);
  renderer_set_palette_texture(palette_texture);

  vec3_t stencil_quad_vertices[] = {
      {0.f, 0.f, 0.f},
      {0.f, 1.f, 0.f},
      {1.f, 1.f, 0.f},
      {1.f, 0.f, 0.f},
  };

  uint32_t stencil_quad_indices[] = {0, 2, 1, 0, 3, 2};

  mesh_create(&quad_mesh, VERTEX_LAYOUT_PLAIN, 4, stencil_quad_vertices, 6,
              stencil_quad_indices, false);
 
  init_weapon_system();
  init_enemy_system();
  init_hud();
  init_bullet_system();
  player_health = 100;
}
bool is_valid_sector(sector_t *sector) {
  ptrdiff_t index = sector - map.sectors;
  return index >= 0 && index < map.num_sectors;
}
static int palette_index = 0;
void       engine_update(float dt) {
  bool noClipoff = true;
  if (is_button_just_pressed(KEY_O)) { palette_index--; }
  if (is_button_just_pressed(KEY_P)) { palette_index++; }

  palette_index = min(max(palette_index, 0), num_palettes - 1);

  camera_update_direction_vectors(&camera);

  vec2_t    position = {camera.position.x, camera.position.z};
  sector_t *sector   = map_get_sector(position);

  if (sector && is_valid_sector(sector)) {
    camera.position.y = sector->floor + player_height;
  } else {
    printf("Warning: invalid or null sector for camera placement.\n");
  }

  float speed =
      (is_button_pressed(KEY_LSHIFT) ? PLAYER_SPEED * 2.f : PLAYER_SPEED) * dt;

  vec3_t forward = camera.forward, right = camera.right;
  forward.y = right.y = 0.f;
  forward             = vec3_normalize(forward);
  right               = vec3_normalize(right);

 vec3_t attempted_pos;

  if (is_button_pressed(KEY_W) || is_button_pressed(KEY_UP)) {
    attempted_pos   = vec3_add(camera.position, vec3_scale(forward, speed));
    vec2_t next_pos = {attempted_pos.x, attempted_pos.z};
    if (is_walkable(next_pos, attempted_pos.y)) camera.position = attempted_pos;
  }

  if (is_button_pressed(KEY_S) || is_button_pressed(KEY_DOWN)) {
    attempted_pos   = vec3_add(camera.position, vec3_scale(forward, -speed));
    vec2_t next_pos = {attempted_pos.x, attempted_pos.z};
    if (is_walkable(next_pos, attempted_pos.y)) camera.position = attempted_pos;
  }

  if (is_button_pressed(KEY_D)) {
    attempted_pos   = vec3_add(camera.position, vec3_scale(right, speed));
    vec2_t next_pos = {attempted_pos.x, attempted_pos.z};
    if (is_walkable(next_pos, attempted_pos.y)) camera.position = attempted_pos;
  }

  if (is_button_pressed(KEY_A)) {
    attempted_pos   = vec3_add(camera.position, vec3_scale(right, -speed));
    vec2_t next_pos = {attempted_pos.x, attempted_pos.z};
    if (is_walkable(next_pos, attempted_pos.y)) camera.position = attempted_pos;
  }
  static bool initialized = false;

  sector_t *sector2 =
      find_sector_for_position((vec2_t){camera.position.x, camera.position.z});
  float floor_height = sector2->floor + 35.0f; // Slight buffer
 bool is_n_pressed = is_button_pressed(KEY_N);

// Toggle noclip when N is pressed (on key down, not held)
if (is_n_pressed && !was_n_pressed) {
  noclip_enabled = !noclip_enabled;
}

was_n_pressed = is_n_pressed;

if (noclip_enabled) {
  if (is_button_pressed(KEY_SPACE)) { camera.position.y += speed; }
  if (is_button_pressed(KEY_R)) {
    camera.position.y -= speed;
  }
}
  
  // On first update, snap to floor and skip jump/gravity logic
  if (!noclip_enabled) {
    if (!initialized) {
      camera.position.y = floor_height;
      camera_y_velocity = 0.0f;
      is_jumping        = false;
      initialized       = true;
    } else {
      // Jump
      if (!is_jumping && is_button_pressed(KEY_SPACE)) {
        is_jumping        = true;
        camera_y_velocity = jump_strength;
      }

      // Apply gravity
      camera_y_velocity += gravity * dt;
      camera.position.y += camera_y_velocity * dt;


       if (camera.position.y >= sector2->ceiling) {
        camera.position.y = sector2->ceiling;
        if (camera_y_velocity > 0.0f) camera_y_velocity = 0.0f;
      }

      // Land
      if (camera.position.y <= floor_height) {
        camera.position.y = floor_height;
        camera_y_velocity = 0.0f;
        is_jumping        = false;
      }
    }
  }
  if (is_button_pressed(KEY_Q)) { camera.position.y += speed; }
  if (is_button_pressed(KEY_N)) {
    // Move up (SPACE)
    if (is_button_pressed(KEY_SPACE)) { camera.position.y += speed; }

    // Move down (CTRL)
    if (is_button_pressed(KEY_R)) { camera.position.y -= speed; }
  }
 
 
  

  float turn_speed = 4.f * dt;
  if (is_button_pressed(KEY_1)) { switch_weapon(0); }
  if (is_button_pressed(KEY_2)) { switch_weapon(1); }
  if (is_button_pressed(KEY_RIGHT)) { camera.yaw -= turn_speed; }
  if (is_button_pressed(KEY_LEFT)) { camera.yaw += turn_speed; }
  if (is_button_pressed(KEY_3)) { fire_bullet();
  }

  if (is_button_pressed(MOUSE_RIGHT)) { set_mouse_captured(1); }

  if (is_button_pressed(KEY_EQUAL)) { camera.pitch = 0.f; }
 

  vec2_t mouse_pos  = get_mouse_position();
  bool   mouse_down = is_button_just_pressed(MOUSE_LEFT);

  switch (current_game_state) {
  case GAME_STATE_START_MENU:
    update_menu(&start_menu, mouse_pos, mouse_down);
    if (start_menu.buttons[0].was_clicked) {
      current_game_state = GAME_STATE_PLAYING;
      set_mouse_captured(1);
    }
    return;

  case GAME_STATE_PAUSED:
    update_menu(&pause_menu, mouse_pos, mouse_down);
    if (pause_menu.buttons[0].was_clicked) {
      current_game_state = GAME_STATE_PLAYING;
      set_mouse_captured(1);
    }
    return;

  case GAME_STATE_PLAYING:
    if (is_button_just_pressed(KEY_ESCAPE)) {
      current_game_state = GAME_STATE_PAUSED;
      set_mouse_captured(0);
      return;
    }
    break;
  }


  static bool is_first = true;
  if (is_mouse_captured()) {
    if (is_first) {
      last_mouse = get_mouse_position();
      is_first   = false;
    }

    vec2_t current_mouse = get_mouse_position();
    float  dx            = current_mouse.x - last_mouse.x;
    float  dy            = last_mouse.y - current_mouse.y;
    last_mouse           = current_mouse;

    camera.yaw -= dx * MOUSE_SENSITIVITY * dt;
    camera.pitch += dy * MOUSE_SENSITIVITY * dt;

    camera.pitch = max(-M_PI_2 + 0.05, min(M_PI_2 - 0.05, camera.pitch));
  } else {
    is_first = true;
  }
  update_bullet_system(dt);
  update_weapon_system(dt);
  update_enemy_system(dt);
  update_hud(dt);

  // Damage flash countdown
  if (damage_flash_timer > 0.0f) { damage_flash_timer -= dt; }

  update_animation(dt);
}
void draw_damage_flash() {
  if (damage_flash_timer <= 0.0f) return;

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, FRAMEBUFFER_WIDTH, 0, FRAMEBUFFER_HEIGHT, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glBegin(GL_QUADS);
  glColor4f(1.0f, 0.0f, 0.0f, damage_flash_timer * 2.0f);
  glVertex2f(0, 0);
  glVertex2f(FRAMEBUFFER_WIDTH, 0);
  glVertex2f(FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
  glVertex2f(0, FRAMEBUFFER_HEIGHT);
  glEnd();

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
}


// Assumes you have a draw function like this elsewhere:
static texture_t load_texture2(const char *path) {
  texture_t tex = {0};

  int            channels;
  unsigned char *data = stbi_load(path, &tex.width, &tex.height, &channels, 4);

  if (!data) {
    printf("Failed to load texture: %s\n", path);
    return tex;
  }

  glGenTextures(1, &tex.id);
  glBindTexture(GL_TEXTURE_2D, tex.id);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width, tex.height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  stbi_image_free(data);
  return tex;
}

 void generate_quad3(vec2_t center, vec2_t size, int texture_id) {
  float x = center.x;
  float y = center.y;
  float w = size.x * 0.5f;
  float h = size.y * 0.5f;

  vertex1_t vertices[6];

  // Triangle 1
  vertices[0].position = (vec3_t){x - w, y - h, 0.0f};
  vertices[0].uv       = (vec2_t){0.0f, 1.0f};
  vertices[1].position = (vec3_t){x + w, y - h, 0.0f};
  vertices[1].uv       = (vec2_t){1.0f, 1.0f};
  vertices[2].position = (vec3_t){x + w, y + h, 0.0f};
  vertices[2].uv       = (vec2_t){1.0f, 0.0f};

  // Triangle 2
  vertices[3].position = (vec3_t){x - w, y - h, 0.0f};
  vertices[3].uv       = (vec2_t){0.0f, 1.0f};
  vertices[4].position = (vec3_t){x + w, y + h, 0.0f};
  vertices[4].uv       = (vec2_t){1.0f, 0.0f};
  vertices[5].position = (vec3_t){x - w, y + h, 0.0f};
  vertices[5].uv       = (vec2_t){0.0f, 0.0f};

  draw_textured_triangles2(vertices, 6, texture_id);
}

static void draw_textured_triangles2(vertex1_t *vertices, int count,
                                    int texture_id) {
  // Bind texture
  glBindTexture(GL_TEXTURE_2D, texture_id);

  glBegin(GL_TRIANGLES);
  for (int i = 0; i < count; i++) {
    glTexCoord2f(vertices[i].uv.x, vertices[i].uv.y);
    glVertex3f(vertices[i].position.x, vertices[i].position.y,
               vertices[i].position.z);
  }
  glEnd();
  glEnd();
}
void draw_textured_quad(GLuint texture) {
  glUseProgram(shader_program);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);

  GLint tex_loc = glGetUniformLocation(shader_program, "u_texture");
  glUniform1i(tex_loc, 0); // texture unit 0

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}
GLuint load_texture(const char *filename) {
  int            width, height, channels;
  
  // Load the image data (force 4 channels: RGBA)
  unsigned char *data = stbi_load(filename, &width, &height, &channels, 4);

  if (!data) {
    printf("Failed to load texture: %s\n", filename);
    return 0;
  }

  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  stbi_image_free(data);
  return tex;
}



void engine_render() {
  GLuint tex;
  stbi_set_flip_vertically_on_load(true); 
  tex = load_texture("start.jpg");
  GLuint tex2 = load_texture("pause.jpg");
  init_shader_program();
  init_quad(-0.5f, -0.5f, 1.0f, 1.0f);
  if (current_game_state == GAME_STATE_START_MENU) {
    draw_textured_quad(tex);
  
  } else if (current_game_state == GAME_STATE_PAUSED) {

     draw_textured_quad(tex2);
   
  } else {
    mat4_t view = mat4_look_at(
        camera.position, vec3_add(camera.position, camera.forward), camera.up);
    renderer_set_view(view);

    renderer_set_palette_index(palette_index);

    glStencilMask(0x00);
    render_node(root_draw_node);

    glStencilMask(0xff);
    for (stencil_node_t *node = stencil_list.head; node != NULL;
         node                 = node->next) {
      renderer_draw_mesh(&quad_mesh, SHADER_PLAIN, node->transformation);
    }

    renderer_draw_sky();
   
    draw_weapon();
    draw_enemies();
    draw_hud();
    draw_damage_flash();
    draw_bullets();
  }
 
}

void render_node(draw_node_t *node) {
  if (node->mesh) {
    renderer_draw_mesh(node->mesh, SHADER_DEFAULT, mat4_identity());
  }

  if (node->front) { render_node(node->front); }
  if (node->back) { render_node(node->back); }
}
bool is_walkable(vec2_t new_pos, float eye_y) {
  for (int i = 0; i < map.num_linedefs; i++) {
    linedef_t *linedef = &map.linedefs[i];

    // Only block if there's no back sidedef (1-sided wall)
    if (linedef->back_sidedef == 0xFFFF) {
      vec2_t p1 = map.vertices[linedef->start_idx];
      vec2_t p2 = map.vertices[linedef->end_idx];

      vec2_t line     = vec2_sub(p2, p1);
      vec2_t to_point = vec2_sub(new_pos, p1);
      float  line_len = vec2_length(line);
      if (line_len < 1e-6f) continue;

      float t =
          clamp(vec2_dot(to_point, line) / (line_len * line_len), 0.f, 1.f);
      vec2_t closest = vec2_add(p1, vec2_scale(line, t));
      float  dist    = vec2_length(vec2_sub(new_pos, closest));

      if (dist < 16.f) {
        return false; // too close to solid wall
      }
    }
  }

  // Check height vs floor/ceiling of sector
  sector_t *sector = find_sector_for_position(new_pos);
  if (sector) {
    if (eye_y < sector->floor ||eye_y> sector->ceiling) { return false; }
  }

  return true;
}

static sector_t *find_sector_for_position(vec2_t pos) {
    for (int i = 0; i < map.num_sectors; i++) {
      sector_t *sector = &map.sectors[i];
      bool      inside = true;

      for (int j = 0; j < map.num_linedefs; j++) {
        linedef_t *line = &map.linedefs[j];

        if (line->front_sidedef != i) continue;

        vec2_t a        = map.vertices[line->start_idx];
        vec2_t b        = map.vertices[line->end_idx];
        vec2_t edge     = vec2_sub(b, a);
        vec2_t to_point = vec2_sub(pos, a);
        vec2_t normal   = (vec2_t){-edge.y, edge.x};

        // If outside one wall, mark as not inside
        if (vec2_dot(normal, to_point) < 0) { inside = false; }
      }

      if (inside) return sector;
    }

    return NULL;
  }
