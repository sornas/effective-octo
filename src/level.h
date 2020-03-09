#pragma once
#include <fog.h>

typedef struct {
    u32 num_points;
    Vec2 *points;
    Vec2 *directions;

    f32 smoothness;
} LevelPointList;

typedef struct {
    f32 width;
    u32 num_checkpoints;
    // TODO(ed): Store forward here, to make sure you can't reverse over it.
    Vec2 *checkpoints;

    u32 num_track_points;
    Vec2 *points;
    u32 num_right_edges;
    Vec2 *right_edges;
    u32 num_left_edges;
    Vec2 *left_edges;
} LevelEdges;

typedef struct {
    u32 num_checkpoints;
    Vec2 *checkpoints;

    u32 num_bodies;
    Body *bodies;
} Level;


LevelPointList generate_level_point_list(f32 noise, f32 offset, f32 smoothness);
void draw_level_point_list(LevelPointList *list);
void clear_level_point_list(LevelPointList *list);

LevelEdges expand_to_edges(LevelPointList *list);
void draw_level_edge(LevelEdges *edge);
void clear_level_edge(LevelEdges *edge);

Level expand_to_level(LevelEdges *level, ShapeID shape);
void draw_level(Level *edge);
void clear_level(Level *edge);
