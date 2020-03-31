#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "level.h"
#include "car.h"
#include "math.h"
#include "game.h"

#define PI 3.1415

static Vec2 from_polar(f32 angle, f32 distance) {
    return fog_V2(distance * cos(angle), distance * sin(angle));
}

// Swapping between generator functions
#if 1
// These are kinda funky, and cool
static f32 distance_func(f32 t, f32 noise) {
    return sin(noise * t) + sin(noise * 0.5 * t) + 10.0;
}

static f32 angle_func(f32 t) {
    return t + t * sin(t) * 0.1;
}
#else
// These give you more straights
static f32 distance_func(f32 t, f32 noise) {
    return cos(noise * t) + sin(noise * 0.333 * t) + 10.0;
}

static f32 angle_func(f32 t) {
    return (t + t * t * 0.1) / 2.0;
}
#endif

static Vec2 calculate_point(f32 t, f32 noise) {
    f32 angle = angle_func(t);
    f32 distance = distance_func(t, noise);
    Vec2 point = from_polar(angle, distance);
    return point;
}

LevelSketch level_gen_sketch(f32 noise, f32 offset, f32 smoothness) {
    u32 num_samples = 8;
    f32 step = 2 * PI / (f32) num_samples;

    LevelSketch point_list = {
        .num_points = num_samples,
        .points = malloc(num_samples * sizeof(Vec2)),
        .directions = malloc(num_samples * sizeof(Vec2)),
        .smoothness = smoothness,
    };

    for (u32 i = 0; i < num_samples; i++) {
        f32 t = step * i + offset;
        Vec2 p = calculate_point(t, noise);
        Vec2 start = calculate_point(t - step * 0.1, noise);
        Vec2 end = calculate_point(t + step * 0.1, noise);
        point_list.points[i] = p;
        point_list.directions[i] = fog_normalize_v2(fog_sub_v2(end, start));
    }
    return point_list;
}

void level_clear_sketch(LevelSketch *sketch) {
    if (sketch->points)
        free(sketch->points);
    if (sketch->directions)
        free(sketch->directions);
    sketch->num_points = 0;
}

static Vec2 sample_from_smoothness(Vec2 p1, Vec2 p2, Vec2 d1, Vec2 d2,
                                   f32 smoothness, f32 t) {
    f32 x = fog_std_progress_func_f32(p1.x, d1.x * smoothness,
                                      p2.x, d2.x * smoothness, t);
    f32 y = fog_std_progress_func_f32(p1.y, d1.y * smoothness, p2.y,
                                      d2.y * smoothness, t);
    return fog_V2(x, y);
}

// Spam points
// Filter the empty ones
LevelBlueprint level_expand_sketch(LevelSketch *sketch, f32 width, f32 spacing,
                                   f32 border_width) {
    assert(spacing >= 0.01 && spacing <= 0.8);
    f32 smoothness = sketch->smoothness;

    u32 maximum_num_points = sketch->num_points * (1.0 / spacing) * 4;
    LevelBlueprint bp = {
        .width = width,
        .border_width = border_width,
        .num_checkpoints = sketch->num_points,
        .checkpoints = malloc(sketch->num_points * sizeof(Vec2)),
        .checkpoints_dir = malloc(sketch->num_points * sizeof(Vec2)),
        .num_track_points = 0,
        .points = malloc(maximum_num_points * sizeof(Vec2)),
        .num_right_edges = 0,
        .right_edges = malloc(maximum_num_points * sizeof(Vec2)),
        .num_left_edges = 0,
        .left_edges = malloc(maximum_num_points * sizeof(Vec2)),
    };

    for (u32 i = 0; i < sketch->num_points; i++) {
        bp.checkpoints[i] = sketch->points[i];
        bp.checkpoints_dir[i] = sketch->directions[i];
    }

    for (u32 i = 0; i < sketch->num_points; i++) {
        Vec2 p1 = sketch->points[i];
        Vec2 d1 = sketch->directions[i];

        u32 j = (i + 1) % sketch->num_points;
        Vec2 p2 = sketch->points[j];
        Vec2 d2 = sketch->directions[j];
        for (f32 t = 0; t < 1.0; t += spacing) {
            Vec2 p = sample_from_smoothness(p1, p2, d1, d2, smoothness, t);
            Vec2 side;
            {
                Vec2 start = sample_from_smoothness(p1, p2, d1, d2,
                                                    smoothness, t - 0.005);
                Vec2 end = sample_from_smoothness(p1, p2, d1, d2,
                                                  smoothness, t + 0.005);
                Vec2 forward = fog_normalize_v2(fog_sub_v2(end, start));
                side = fog_mul_v2(fog_rotate_ccw_v2(forward), width);
            }
            Vec2 left_side = fog_sub_v2(p, side);
            Vec2 right_side = fog_add_v2(p, side);

            bp.points[bp.num_track_points++] = p;
            bp.left_edges[bp.num_left_edges++] = left_side;
            bp.right_edges[bp.num_right_edges++] = right_side;
        }
    }

    for (u32 i = 0; i < bp.num_track_points; i++) {
        Vec2 p = bp.points[i];

        u32 removed_right = 0;
        for (u32 j = 0; j < bp.num_right_edges; j++) {
            Vec2 e = bp.right_edges[j];
            Vec2 a = fog_V2(e.x, e.y);
            if (fog_distance_v2(p, a) < bp.width - 0.01) {
                removed_right++;
                continue;
            }
            assert(removed_right <= j);
            bp.right_edges[j - removed_right] = e;
        }
        bp.num_right_edges -= removed_right;

        u32 removed_left = 0;
        for (u32 j = 0; j < bp.num_left_edges; j++) {
            Vec2 e = bp.left_edges[j];
            Vec2 a = fog_V2(e.x, e.y);
            if (fog_distance_v2(p, a) < bp.width - 0.01) {
                removed_left++;
                continue;
            }
            assert(removed_left <= j);
            bp.left_edges[j - removed_left] = e;
        }
        bp.num_left_edges -= removed_left;
    }

    return bp;
}

void level_draw_blueprint(LevelBlueprint *bp) {
    Vec2 a;
    Vec2 b;

    for (u32 i = 0; i < bp->num_track_points; i++) {
        u32 j = (i + 1) % bp->num_track_points;
        a = bp->points[i];
        b = bp->points[j];
        fog_renderer_push_line(0, a, b, fog_V4(1, 0, 0, 0.5), 0.05);
    }

    for (u32 i = 0; i < bp->num_right_edges; i++) {
        u32 j = (i + 1) % bp->num_right_edges;
        a = bp->right_edges[i];
        b = bp->right_edges[j];
        fog_renderer_push_line(0, a, b, fog_V4(0, 1, 0, 0.5), 0.05);
    }

    for (u32 i = 0; i < bp->num_left_edges; i++) {
        u32 j = (i + 1) % bp->num_left_edges;
        a = bp->left_edges[i];
        b = bp->left_edges[j];
        fog_renderer_push_line(0, a, b, fog_V4(0, 0, 1, 0.5), 0.05);
    }
}

void level_clear_blueprint(LevelBlueprint *bp) {
    bp->num_checkpoints = 0;
    bp->num_track_points = 0;
    bp->num_right_edges = 0;
    bp->num_left_edges = 0;
    if (bp->checkpoints) {
        free(bp->checkpoints);
        free(bp->checkpoints_dir);
    }
    if (bp->points) {
        free(bp->points);
        free(bp->right_edges);
        free(bp->left_edges);
    }
}

void level_draw_sketch(LevelSketch *sketch) {
    f32 width = 0.5;
    f32 spacing = 0.01;
    f32 smoothness = sketch->smoothness;
    // fog_util_tweak_f32("Smoothness", &smoothness, 0.4);
    for (u32 i = 0; i < sketch->num_points; i++) {
        Vec2 p1 = sketch->points[i];
        Vec2 d1 = sketch->directions[i];

        u32 j = (i + 1) % sketch->num_points;
        Vec2 p2 = sketch->points[j];
        Vec2 d2 = sketch->directions[j];
        for (f32 t = 0; t < 1.0; t += spacing) {
            Vec2 p = sample_from_smoothness(p1, p2, d1, d2, smoothness, t);
            Vec2 start = sample_from_smoothness(p1, p2, d1, d2, smoothness, t - 0.005);
            Vec2 end = sample_from_smoothness(p1, p2, d1, d2, smoothness, t + 0.005);
            Vec2 forward = fog_normalize_v2(fog_sub_v2(end, start));
            Vec2 side = fog_mul_v2(fog_rotate_ccw_v2(forward), width);
            fog_renderer_push_point(1, p, fog_V4(0, 0, 1, 1), 0.04);
            fog_renderer_push_line(0, p, fog_add_v2(p, forward), fog_V4(1, 0, 1, 0.2), 0.04);
            fog_renderer_push_line(1, fog_add_v2(p, side), fog_sub_v2(p, side), fog_V4(0, 0, 1, 0.3), 0.04);
        }

        // fog_renderer_push_point(1, p, fog_V4(0, 0, 1, 1), 0.05);
        fog_renderer_push_line(0, p1, fog_add_v2(p1, d1), fog_V4(1, 0, 0, 1), 0.03);

    }
}

Level level_expand(LevelBlueprint *bp, ShapeID shape) {
    u32 max_num_bodies = bp->num_right_edges + bp->num_left_edges;
    Body *bodies = malloc(max_num_bodies * sizeof(Body));

    u32 num_bodies = 0;
    for (u32 i = 0; i < bp->num_left_edges; i++) {
        u32 j = (i + 1) % bp->num_left_edges;
        Vec2 a = bp->left_edges[i];
        Vec2 b = bp->left_edges[j];
        Body body = fog_physics_create_body(shape, 0, 0.0, 0.0);
        body.position = fog_mul_v2(fog_add_v2(a, b), 0.5);
        body.rotation = fog_angle_v2(fog_sub_v2(b, a));
        body.scale.y = bp->border_width;
        body.scale.x = fog_distance_v2(a, b);
        assert(num_bodies < max_num_bodies);
        bodies[num_bodies++] = body;
    }

    for (u32 i = 0; i < bp->num_right_edges; i++) {
        u32 j = (i + 1) % bp->num_right_edges;
        Vec2 a = bp->right_edges[i];
        Vec2 b = bp->right_edges[j];
        Body body = fog_physics_create_body(shape, 0, 0.0, 0.0);
        body.position = fog_mul_v2(fog_add_v2(a, b), 0.5);
        body.rotation = fog_angle_v2(fog_sub_v2(b, a));
        body.scale.y = bp->border_width;
        body.scale.x = fog_distance_v2(a, b);
        assert(num_bodies < max_num_bodies);
        bodies[num_bodies++] = body;
    }

    const u32 num_trees = 4000;
    Level level = {
        .num_checkpoints = bp->num_checkpoints,
        .checkpoints = malloc(bp->num_checkpoints * sizeof(Body)),
        .checkpoints_dir = malloc(bp->num_checkpoints * sizeof(Body)),

        .num_bodies = num_bodies,
        .bodies = bodies,

        .num_trees = num_trees,
        .trees = malloc(num_trees * sizeof(Vec2)),
        .sizes = malloc(num_trees * sizeof(f32)),

        .width = bp->width,
    };

    for (u32 i = 0; i < bp->num_checkpoints; i++) {
        level.checkpoints[i] = bp->checkpoints[i];
        level.checkpoints_dir[i] = bp->checkpoints_dir[i];
    }

    for (u32 i = 0; i < level.num_trees / 2; i++) {
        Vec2 start;
        Vec2 end;

        start = bp->right_edges[(i + 0) % bp->num_right_edges];
        end = bp->right_edges[(i + 1) % bp->num_right_edges];
#define LERP_VEC(a, b, l) fog_add_v2(fog_add_v2(fog_mul_v2(a, 1.0 - l), fog_mul_v2(b, l)), fog_random_unit_vec2())
        f32 l = fog_random_real(0, 1);
        level.trees[i * 2 + 0] = LERP_VEC(start, end, l);
        level.sizes[i * 2 + 0] = fog_random_real(2.0, 3.0);

        l = fog_random_real(0, 1);
        start = bp->left_edges[(i + 0) % bp->num_left_edges];
        end = bp->left_edges[(i + 1) % bp->num_left_edges];
        level.trees[i * 2 + 1] = LERP_VEC(start, end, l);
        level.sizes[i * 2 + 1] = fog_random_real(2.0, 3.0);
    }

    return level;
}

void level_draw(Level *level) {
    for (u32 i = 0; i < level->num_bodies; i++) {
        fog_physics_debug_draw_body(level->bodies + i);
    }

    for (u32 i = 0; i < level->num_checkpoints; i++) {
        Vec2 p = level->checkpoints[i];
        Vec2 d = level->checkpoints_dir[i];
        fog_renderer_push_point(0, p, fog_V4(0, 1, 0, 1), 0.1);
        fog_renderer_push_line(0, p, fog_add_v2(p, d), fog_V4(0, 1, 0, 1), 0.1);
    }

    for (u32 i = 0; i < level->num_trees; i++) {
        Vec2 scale = fog_V2(level->sizes[i], level->sizes[i]);
        fog_renderer_push_sprite(3, PINE_SPRITES[i % NUM_PINE_SPRITES], level->trees[i], scale, 0, fog_V4(1, 1, 1, 1));
    }
}

void level_place(Level *level, struct Car *car) {
    Vec2 p = level->checkpoints[0];
    Vec2 d = level->checkpoints_dir[0];
    f32 angle = fog_angle_v2(d);
    car->body.rotation = angle;

    switch (car->player) {
    case P1:
        car->body.position = fog_add_v2(p, fog_rotate_v2(fog_mul_v2(fog_V2(-level->width, 0), 0.25), angle - PI/2));
        break;
    case P2:
        car->body.position = fog_add_v2(p, fog_rotate_v2(fog_mul_v2(fog_V2(level->width, 0), 0.25), angle - PI/2));
        break;
    default:
        break;
    }
    reset_car(car, level->num_checkpoints);
}

void level_clear(Level *level) {
    if (level->bodies) {
        free(level->bodies);
        free(level->checkpoints);
        free(level->checkpoints_dir);
        free(level->trees);
    }
}

Level level_gen(f32 noise, f32 offset, f32 smoothness, f32 width, f32 spacing, f32 border_width, ShapeID shape) {
    LevelSketch lvl_sketch = level_gen_sketch(noise, offset, smoothness);
    LevelBlueprint lvl_bp =
        level_expand_sketch(&lvl_sketch, width, spacing, border_width);
    Level lvl = level_expand(&lvl_bp, shape);
    level_clear_sketch(&lvl_sketch);
    level_clear_blueprint(&lvl_bp);
    return lvl;
}
