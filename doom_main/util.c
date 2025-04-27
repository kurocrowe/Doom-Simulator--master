#include "util.h"
#include "map.h"
#include "matrix.h"
#include "state.h"
#include "vector.h"

#include <stdbool.h>

void insert_stencil_quad(mat4_t transformation) {
  if (stencil_list.head == NULL) {
    stencil_list.head  = malloc(sizeof(stencil_node_t));
    *stencil_list.head = (stencil_node_t){transformation, NULL};

    stencil_list.tail = stencil_list.head;
  } else {
    stencil_list.tail->next  = malloc(sizeof(stencil_node_t));
    *stencil_list.tail->next = (stencil_node_t){transformation, NULL};

    stencil_list.tail = stencil_list.tail->next;
  }
}

sector_t *map_get_sector(vec2_t position) {
  printf("map_get_sector called with position: (%f, %f)\n", position.x,
         position.y);

  uint16_t id = gl_map.num_nodes - 1;
  while ((id & 0x8000) == 0) {
    if (id >= gl_map.num_nodes) {
      printf("Invalid node id: %u\n", id);
      return NULL;
    }

    gl_node_t *node       = &gl_map.nodes[id];
    vec2_t     delta      = vec2_sub(position, node->partition);
    bool       is_on_back = (delta.x * node->delta_partition.y -
                       delta.y * node->delta_partition.x) <= 0.f;

    id = is_on_back ? node->back_child_id : node->front_child_id;
  }

  uint16_t ssec_index = id & 0x7fff;
  printf("Selected subsector index: %u\n", ssec_index);

  if (ssec_index >= gl_map.num_subsectors) {
    printf("Invalid subsector index: %u\n", ssec_index);
    return NULL;
  }

  gl_subsector_t *subsector = &gl_map.subsectors[ssec_index];

  if (subsector->first_seg >= gl_map.num_segments) {
    printf("Invalid first_seg index: %u\n", subsector->first_seg);
    return NULL;
  }

  gl_segment_t *segment = &gl_map.segments[subsector->first_seg];
  printf("Segment linedef: %u, side: %u\n", segment->linedef, segment->side);

  if (segment->linedef >= map.num_linedefs) {
    printf("Invalid linedef index: %u\n", segment->linedef);
    return NULL;
  }

  if (segment->side != 0 && segment->side != 1) {
    printf("Invalid segment side: %u\n", segment->side);
    return NULL;
  }

  linedef_t *linedef = &map.linedefs[segment->linedef];
  sidedef_t *sidedef;

  if (segment->side == 0) {
    if (linedef->front_sidedef >= map.num_sidedefs) {
      printf("Invalid front_sidedef index: %u\n", linedef->front_sidedef);
      return NULL;
    }
    sidedef = &map.sidedefs[linedef->front_sidedef];
  } else {
    if (linedef->back_sidedef >= map.num_sidedefs) {
      printf("Invalid back_sidedef index: %u\n", linedef->back_sidedef);
      return NULL;
    }
    sidedef = &map.sidedefs[linedef->back_sidedef];
  }

  if (sidedef->sector_idx >= map.num_sectors) {
    printf("Invalid sector index: %u\n", sidedef->sector_idx);
    return NULL;
  }

  return &map.sectors[sidedef->sector_idx];
}
