#ifndef BSP_H
#define BSP_H

#include <stdint.h>

typedef struct {
  int16_t  x, y;
  int16_t  dx, dy;
  uint16_t front_bbox[4];
  uint16_t back_bbox[4];
  uint16_t front_id;
  uint16_t back_id;
} bsp_node_t;

#endif // BSP_H
