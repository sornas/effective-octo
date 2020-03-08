#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

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
LevelEdges expand_to_edges(LevelPointList *list) {
    static f32 width = 0.5;
    fog_util_tweak_f32("Width", &width, 0.1);
    static f32 spacing = 0.2;
    fog_util_tweak_f32("Spacing", &spacing, 0.01);
    if (spacing < 0.01) {
        spacing = 0.01;
    }
    if (spacing > 0.8) {
        spacing = 0.8;
    }
    f32 smoothness = list->smoothness;

    u32 maximum_num_points = list->num_points * (1.0 / spacing) * 4;
    LevelEdges edge = {
        .width = width,
        .num_checkpoints = 0,
        .num_track_points = 0,
        .points = malloc(maximum_num_points * sizeof(Vec2)),
        .num_right_edges = 0,
        .right_edges = malloc(maximum_num_points * sizeof(Vec2)),
        .num_left_edges = 0,
        .left_edges = malloc(maximum_num_points * sizeof(Vec2)),
    };

    for (u32 i = 0; i < list->num_points; i++) {
        Vec2 p1 = list->points[i];
        Vec2 d1 = list->directions[i];

        u32 j = (i + 1) % list->num_points;
        Vec2 p2 = list->points[j];
        Vec2 d2 = list->directions[j];
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

            edge.points[edge.num_track_points++] = p;
            edge.left_edges[edge.num_left_edges++] = left_side;
            edge.right_edges[edge.num_right_edges++] = right_side;
        }
    }

    for (u32 i = 0; i < edge.num_track_points; i++) {
        Vec2 p = edge.points[i];

        u32 removed_right = 0;
        for (u32 j = 0; j < edge.num_right_edges; j++) {
            Vec2 a = edge.right_edges[j];
            if (fog_distance_v2(p, a) < edge.width - 0.01) {
                removed_right++;
                continue;
            }
            assert(removed_right <= j);
            edge.right_edges[j - removed_right] = a;
        }
        edge.num_right_edges -= removed_right;

        u32 removed_left = 0;
        for (u32 j = 0; j < edge.num_left_edges; j++) {
            Vec2 a = edge.left_edges[j];
            if (fog_distance_v2(p, a) < edge.width - 0.01) {
                removed_left++;
                continue;
            }
            assert(removed_left <= j);
            edge.left_edges[j - removed_left] = a;
        }
        edge.num_left_edges -= removed_left;
    }

    return edge;
}

void draw_level_edge(LevelEdges *edge) {
    Vec2 a;
    Vec2 b;

    for (u32 i = 0; i < edge->num_track_points; i++) {
        u32 j = (i + 1) % edge->num_track_points;
        a = edge->points[i];
        b = edge->points[j];
        fog_renderer_push_line(0, a, b, fog_V4(1, 0, 0, 0.5), 0.05);
    }

    for (u32 i = 0; i < edge->num_right_edges; i++) {
        u32 j = (i + 1) % edge->num_right_edges;
        a = edge->right_edges[i];
        b = edge->right_edges[j];
        fog_renderer_push_line(0, a, b, fog_V4(0, 1, 0, 0.5), 0.05);
    }

    for (u32 i = 0; i < edge->num_left_edges; i++) {
        u32 j = (i + 1) % edge->num_left_edges;
        a = edge->left_edges[i];
        b = edge->left_edges[j];
        fog_renderer_push_line(0, a, b, fog_V4(0, 0, 1, 0.5), 0.05);
    }
}

void clear_level_edge(LevelEdges *edge) {
    edge->num_checkpoints = 0;
    edge->num_track_points = 0;
    if (edge->checkpoints) {
        free(edge->checkpoints);
    }
    if (edge->points) {
        free(edge->points);
        free(edge->right_edges);
        free(edge->left_edges);
    }
}

void draw_level_point_list(LevelPointList *list) {
    static f32 width = 0.5;
    fog_util_tweak_f32("Width", &width, 0.1);
    static f32 spacing = 0.2;
    fog_util_tweak_f32("Spacing", &spacing, 0.01);
    if (spacing < 0.01) {
        spacing = 0.01;
    }
    if (spacing > 0.8) {
        spacing = 0.8;
    }
    f32 smoothness = list->smoothness;
    // fog_util_tweak_f32("Smoothness", &smoothness, 0.4);
    for (u32 i = 0; i < list->num_points; i++) {
        Vec2 p1 = list->points[i];
        Vec2 d1 = list->directions[i];

        u32 j = (i + 1) % list->num_points;
        Vec2 p2 = list->points[j];
        Vec2 d2 = list->directions[j];
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
