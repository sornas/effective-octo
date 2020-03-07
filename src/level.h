#pragma once
#include <fog.h>

typedef struct {
    u32 num_points;
    Vec2 *points;
    Vec2 *directions;
} LevelPointList;

void draw_level_point_list(LevelPointList *list);

LevelPointList generate_level_point_list();
void clear_level_point_list(LevelPointList *list);

