#include "camera.h"
#include "vector.h"
#include <math.h>
camera_t camera;

void camera_update_direction_vectors(camera_t *camera) {
  camera->forward = (vec3_t){
      cosf(camera->yaw) * cosf(camera->pitch),
      sinf(camera->pitch),
      sinf(camera->yaw) * cosf(camera->pitch),
  };

  camera->plane.x = -camera->forward.z * 0.66f;
  camera->plane.y = camera->forward.x * 0.66f;
  camera->right = vec3_normalize(vec3_cross(WORLD_UP, camera->forward));
  camera->up    = vec3_normalize(vec3_cross(camera->forward, camera->right));
}