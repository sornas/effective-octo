#include <stdio.h>
#include <stdlib.h>
#define FOG_IMPL
#include <fog.h>

#include "game.h"
#include "car.h"
#include "level.h"

u32 num_bodies = 4;
Body *bodies;
Car car;

LevelSketch lvl_sketch = {};
LevelBlueprint lvl_bp = {};
Level lvl = {};

ShapeID square;

void update() {
    update_car(&car, fog_logic_delta());
}

void draw() {
    draw_car(&car);
    // for (u32 i = 0; i < num_bodies; i++) {
    //     fog_physics_debug_draw_body(&bodies[i]);
    // }
    static f32 noise = 2.0;
    static f32 offset = 0.2;
    static f32 smoothness = 3.0;
    static f32 width = 0.4;
    static f32 spacing = 0.10;
    static f32 border_width = 0.1;
    bool change = false;
    change |= fog_util_tweak_f32("Noise", &noise, 0.1);
    change |= fog_util_tweak_f32("Offset", &offset, 0.1);
    change |= fog_util_tweak_f32("Smoothness", &smoothness, 0.1);
    change |= fog_util_tweak_f32("Width", &width, 0.1);
    change |= fog_util_tweak_f32("Spacing", &spacing, 0.1);
    change |= fog_util_tweak_f32("Border Width", &border_width, 0.1);
    static b8 gen_new_track = true;
    fog_util_tweak_b8("Gen new", &gen_new_track);
    if (gen_new_track) {
        noise = fog_random_real(0.2, 5.0);
        offset = fog_random_real(-5.0, 5.0);
        change = true;
        gen_new_track = false;
    }

#if 0
    if (change) {
        level_clear_sketch(&lvl_sketch);
        lvl_sketch = level_gen_sketch(noise, offset, smoothness);
        level_clear_blueprint(&lvl_bp);
        lvl_bp = level_expand_sketch(&lvl_sketch, width, spacing, border_width);
    }
    level_clear(&lvl);
    lvl = level_expand(&lvl_bp, square);

    // draw_level_point_list(&lvl_sketch);
    level_draw_blueprint(&lvl_bp);
    level_draw(&lvl);
#else
    if (change)
        lvl = level_gen(noise, offset, smoothness, width, spacing, border_width, square);
    level_draw(&lvl);
#endif
}

int main(int argc, char **argv) {
    for (u32 i = 0; i < NUM_BINDINGS; i++) {
        bindings[i] = fog_input_request_name(1);
    }

    fog_init(argc, argv);

    fog_input_add(fog_key_to_input_code(SDLK_a), NAME(LEFT), P1);
    fog_input_add(fog_key_to_input_code(SDLK_d), NAME(RIGHT), P1);
    fog_input_add(fog_key_to_input_code(SDLK_w), NAME(FORWARD), P1);
    fog_input_add(fog_key_to_input_code(SDLK_s), NAME(BACKWARD), P1);

    car_sprite = fog_asset_fetch_id("CAR_SPRITE");
    car_shape = fog_physics_add_shape_from_sprite(car_sprite);

    square = car_shape;
    car = create_car(P1);
    //body = fog_physics_create_body(car_shape, 0);
    bodies = malloc(sizeof(Body) * num_bodies);
    for (u32 i = 0; i < num_bodies; i++) {
        bodies[i] = fog_physics_create_body(car_shape, 0, 0.0, 0.0);
        bodies[i].position = fog_random_unit_vec2();
        bodies[i].scale = fog_random_unit_vec2();
    }

    fog_renderer_fetch_camera(0)->zoom = 1.0 / 5.0;

    fog_run(update, draw);

    return 0;
}
