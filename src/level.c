#include <stdlib.h>
#include <stdio.h>

#include "level.h"
#include "math.h"

#define PI 3.1415

static Vec2 from_polar(f32 angle, f32 distance) {
    return fog_V2(distance * cos(angle), distance * sin(angle));
}

static f32 distance_func(f32 t, f32 noise) {
    return sin(noise * t) + sin(noise * 0.5 * t) + 3.0;
}

static f32 angle_func(f32 t) {
    return t + t * sin(t) * 0.1;
}

static Vec2 calculate_point(f32 t, f32 noise) {
    f32 angle = angle_func(t);
    f32 distance = distance_func(t, noise);
    Vec2 point = from_polar(angle, distance);
    return point;
}

LevelPointList generate_level_point_list(f32 noise, f32 offset, f32 smoothness) {
    u32 num_samples = 8;
    f32 step = 2 * PI / (f32) num_samples;

    LevelPointList point_list = {
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

void clear_level_point_list(LevelPointList *list) {
    if (list->points)
        free(list->points);
    if (list->directions)
        free(list->directions);
    list->num_points = 0;
}

void draw_level_point_list(LevelPointList *list) {
    f32 smoothness = list->smoothness;
    // fog_util_tweak_f32("Smoothness", &smoothness, 0.4);
    for (u32 i = 0; i < list->num_points; i++) {
        Vec2 p1 = list->points[i];
        Vec2 d1 = list->directions[i];

        u32 j = (i + 1) % list->num_points;
        Vec2 p2 = list->points[j];
        Vec2 d2 = list->directions[j];
        for (f32 t = 0; t < 1.0; t += 0.01) {
            f32 x = fog_std_progress_func_f32(p1.x, d1.x * smoothness, p2.x, d2.x * smoothness, t);
            f32 y = fog_std_progress_func_f32(p1.y, d1.y * smoothness, p2.y, d2.y * smoothness, t);
            fog_renderer_push_point(1, fog_V2(x, y), fog_V4(0, 0, 1, 1), 0.04);
        }

        // fog_renderer_push_point(1, p, fog_V4(0, 0, 1, 1), 0.05);
        fog_renderer_push_line(0, p1, fog_add_v2(p1, d1), fog_V4(1, 0, 0, 1), 0.03);

    }
}
