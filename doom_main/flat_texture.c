#include "flat_texture.h"
#include <assert.h>
#include <glad/glad.h>
#include <stddef.h>

GLuint generate_flat_texture_array(const flat_tex_t *flats, size_t num_flats) {
  GLuint tex_id;
  glGenTextures(1, &tex_id);
  glBindTexture(GL_TEXTURE_2D_ARRAY, tex_id);

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage3D(GL_TEXTURE_2D_ARRAY,
               0,                  // mipmap level
               GL_R8UI,            // internal format
               FLAT_TEXTURE_SIZE,  // width
               FLAT_TEXTURE_SIZE,  // height
               (GLsizei)num_flats, // layers (depth)
               0,                  // border (must be 0)
               GL_RED_INTEGER,     // format
               GL_UNSIGNED_BYTE,   // data type
               NULL);              // no initial data


  for (size_t i = 0; i < num_flats; i++) {
    assert(flats[i].data); // Ensure data pointer is valid
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                    0,              // mipmap level
                    0, 0, (GLint)i, // x, y, layer
                    FLAT_TEXTURE_SIZE, FLAT_TEXTURE_SIZE,
                    1,              // depth (1 layer)
                    GL_RED_INTEGER, // format must match GL_R8UI
                    GL_UNSIGNED_BYTE, flats[i].data);
  }

  return tex_id;
}
