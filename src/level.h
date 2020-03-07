#pragma once
#include <fog.h>

typedef struct {
    u32 num_points;
    Vec2 *points;
    Vec2 *directions;

    f32 smoothness;
} LevelPointList;

void draw_level_point_list(LevelPointList *list);

LevelPointList generate_level_point_list(f32 noise, f32 offset, f32 smoothness);
void clear_level_point_list(LevelPointList *list);

