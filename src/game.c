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

AssetID PINE_SPRITES[NUM_PINE_SPRITES] = {};

LevelSketch lvl_sketch = {};
LevelBlueprint lvl_bp = {};
Level lvl = {};
b8 level_exists = false;

ShapeID square;

void build_level() {
    static f32 noise = 2.0;
    static f32 offset = 0.2;
    static f32 smoothness = 3.0;
    static f32 width = 0.4;
    static f32 spacing = 0.10;
    static f32 border_width = 0.1;
    bool change = false;

    if (!level_exists) {
        lvl = level_gen(noise, offset, smoothness, width, spacing, border_width,
                        square);
        level_exists = true;
    }

    static b8 track_parameters = 0;
    if (fog_util_begin_tweak_section("track parameters", &track_parameters)) {
        change |= fog_util_tweak_f32("Noise", &noise, 0.1);
        change |= fog_util_tweak_f32("Offset", &offset, 0.1);
        change |= fog_util_tweak_f32("Smoothness", &smoothness, 0.1);
        change |= fog_util_tweak_f32("Width", &width, 0.1);
        change |= fog_util_tweak_f32("Spacing", &spacing, 0.1);
        change |= fog_util_tweak_f32("Border Width", &border_width, 0.1);
        static b8 gen_new_track = false;
        fog_util_tweak_b8("Gen new", &gen_new_track);
        if (gen_new_track) {
            noise = fog_random_real(0.2, 5.0);
            offset = fog_random_real(-5.0, 5.0);
            change = true;
            gen_new_track = false;
        }

        if (change)
            lvl = level_gen(noise, offset, smoothness, width, spacing, border_width,
                            square);
    }
    fog_util_end_tweak_section(&track_parameters);
    level_draw(&lvl);
}

void update() {
    update_car(&car, fog_logic_delta());
    fog_renderer_fetch_camera(0)->position = fog_add_v2(car.body.position, fog_mul_v2(car.body.velocity, 0.01));
    build_level();
}

void draw() {
    draw_car(&car);
    for (u32 i = 0; i < num_bodies; i++) {
        fog_physics_debug_draw_body(&bodies[i]);
    }
    static f32 angle = 0;
    fog_util_tweak_f32("angle", &angle, 0.1);
    //fog_renderer_push_sprite(0, fetch_car_sprite(angle), fog_V2(0, 0), fog_V2(1, 1), 0, fog_V4(1, 1, 1, 1));


    fog_random_seed(0);
    fog_renderer_push_sprite(0, PINE_SPRITES[0], fog_random_unit_vec2(),
                             fog_V2(1, 1), 0, fog_V4(1, 1, 1, 1));
    fog_renderer_push_sprite(0, PINE_SPRITES[1], fog_random_unit_vec2(),
                             fog_V2(1, 1), 0, fog_V4(1, 1, 1, 1));
    fog_renderer_push_sprite(0, PINE_SPRITES[2], fog_random_unit_vec2(),
                             fog_V2(1, 1), 0, fog_V4(1, 1, 1, 1));
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
    fog_input_add(fog_key_to_input_code(SDLK_SPACE), NAME(DRIFT), P1);

    char str[100] = {};
    for (u32 i = 0; i < NUM_CAR_SPRTIES; i++) {
        sprintf(str, "CAR%d", i);
        CAR_SPRITES[i] = fog_asset_fetch_id(str);
    }

    for (u32 i = 0; i < NUM_PINE_SPRITES; i++) {
        sprintf(str, "PINE%d", i);
        PINE_SPRITES[i] = fog_asset_fetch_id(str);
    }

    car_sprite = fog_asset_fetch_id("CAR_SPRITE");
    car_shape = fog_physics_add_shape_from_sprite(car_sprite);
    square = car_shape;

    car = create_car(P1);

    fog_renderer_set_window_size(800, 800);
    fog_renderer_fetch_camera(0)->zoom = 1.0 / 5.0;

    bodies = malloc(sizeof(Body) * num_bodies);
    for (u32 i = 0; i < num_bodies; i++) {
        bodies[i] = fog_physics_create_body(car_shape, 0, 0.0, 0.0);
        bodies[i].position = fog_random_unit_vec2();
        bodies[i].scale = fog_random_unit_vec2();
    }

    fog_run(update, draw);

    return 0;
}
