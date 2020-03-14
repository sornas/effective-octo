#pragma once
#include <fog.h>

typedef struct {
    u32 num_points;
    Vec2 *points;
    Vec2 *directions;

    f32 smoothness;
} LevelSketch;

typedef struct {
    f32 width;
    f32 border_width;

    u32 num_checkpoints;
    // TODO(ed): Store forward here, to make sure you can't reverse over it.
    Vec2 *checkpoints;

    u32 num_track_points;
    Vec2 *points;
    u32 num_right_edges;
    Vec2 *right_edges;
    u32 num_left_edges;
    Vec2 *left_edges;
} LevelBlueprint;

typedef struct {
    u32 num_checkpoints;
    Vec2 *checkpoints;

    u32 num_bodies;
    Body *bodies;
} Level;


// First step in generation.
LevelSketch level_gen_sketch(f32 noise, f32 offset, f32 smoothness);
void level_draw_sketch(LevelSketch *sketch);
void level_clear_sketch(LevelSketch *sketch);

// Solidifes the design even more.
LevelBlueprint level_expand_sketch(LevelSketch *sketch, f32 width, f32 spacing,
                               f32 border_width);
void level_draw_blueprint(LevelBlueprint *bp);
void level_clear_blueprint(LevelBlueprint *bp);

// The final step, adding in the collision shapes and such
Level level_expand(LevelBlueprint *level, ShapeID shape);
void level_draw(Level *level);
void level_clear(Level *level);

// Generate a level from start to finish
Level level_gen(f32 noise, f32 offset, f32 smoothness, f32 width, f32 spacing, f32 border_width, ShapeID shape);
