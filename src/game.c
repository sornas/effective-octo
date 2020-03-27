#include <stdio.h>
#include <stdlib.h>
#define FOG_IMPL
#include <fog.h>

#include "game.h"
#include "car.h"
#include "level.h"

Car car1;
Car car2;

AssetID PINE_SPRITES[NUM_PINE_SPRITES] = {};

LevelSketch lvl_sketch = {};
LevelBlueprint lvl_bp = {};
Level lvl = {};

ShapeID square;

void build_level() {
    static f32 noise = 2.0;
    static f32 offset = 0.2;
    static f32 smoothness = 8.0;
    static f32 width = 1.4;
    static f32 spacing = 0.3;
    static f32 border_width = 0.5;
    static bool change = true;

    static b8 track_parameters = 0;
    if (fog_util_begin_tweak_section("track parameters", &track_parameters)) {
        change |= fog_util_tweak_f32("Noise", &noise, 0.1);
        change |= fog_util_tweak_f32("Offset", &offset, 0.1);
        change |= fog_util_tweak_f32("Smoothness", &smoothness, 0.1);
        change |= fog_util_tweak_f32("Width", &width, 0.1);
        change |= fog_util_tweak_f32("Spacing", &spacing, 0.1);
        spacing = spacing < 0.01 ? 0.01 : spacing;

        change |= fog_util_tweak_f32("Border Width", &border_width, 0.1);

        b8 gen_new_track = false;
        if (change |= fog_util_tweak_b8("Gen new", &gen_new_track)) {
            noise = fog_random_real(0.2, 5.0);
            offset = fog_random_real(-5.0, 5.0);
        }
    }

    if (change) {
        lvl = level_gen(noise, offset, smoothness, width, spacing,
                border_width, square);
        level_place(&lvl, &car1);
        level_place(&lvl, &car2);
        change = false;
    }

    fog_util_end_tweak_section(&track_parameters);
}

void update() {
    static b8 settings = 0;
    if (fog_util_begin_tweak_section("Settings", &settings)) {
        fog_util_tweak_b8("Car 1 - controller", &car1.controller);
        fog_util_tweak_b8("Car 2 - controller", &car2.controller);
    }
    fog_util_end_tweak_section(&settings);

    build_level();
    update_car(&car1, &lvl, fog_logic_delta());
    update_car(&car2, &lvl, fog_logic_delta());

    //TODO(gu) these should ideally not be hard-coded
    fog_renderer_fetch_camera(0)->position = fog_add_v2(
            fog_mul_v2(fog_V2(10, 0), fog_renderer_fetch_camera(0)->zoom),
            fog_add_v2(car1.body.position,
                       fog_mul_v2(car1.body.velocity, 0.01)));
    fog_renderer_fetch_camera(1)->position = fog_add_v2(
            fog_mul_v2(fog_V2(-10, 0), fog_renderer_fetch_camera(1)->zoom),
            fog_add_v2(car2.body.position,
                       fog_mul_v2(car2.body.velocity, 0.01)));
}

void draw() {
    draw_car(&car1);
    draw_car(&car2);

    //fog_random_seed(0);
    //fog_renderer_push_sprite(0, PINE_SPRITES[0], fog_random_unit_vec2(),
    //                         fog_V2(1, 1), 0, fog_V4(1, 1, 1, 1));
    //fog_renderer_push_sprite(0, PINE_SPRITES[1], fog_random_unit_vec2(),
    //                         fog_V2(1, 1), 0, fog_V4(1, 1, 1, 1));
    //fog_renderer_push_sprite(0, PINE_SPRITES[2], fog_random_unit_vec2(),
    //                         fog_V2(1, 1), 0, fog_V4(1, 1, 1, 1));

    level_draw(&lvl);
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
    fog_input_add(fog_key_to_input_code(SDLK_j), NAME(LEFT), P2);
    fog_input_add(fog_key_to_input_code(SDLK_l), NAME(RIGHT), P2);
    fog_input_add(fog_key_to_input_code(SDLK_i), NAME(FORWARD), P2);
    fog_input_add(fog_key_to_input_code(SDLK_k), NAME(BACKWARD), P2);
    fog_input_add(fog_key_to_input_code(SDLK_RSHIFT), NAME(DRIFT), P2);

    fog_input_add(fog_axis_to_input_code(SDL_CONTROLLER_AXIS_LEFTX, 0), NAME(LEFTRIGHT), P1);
    fog_input_add(fog_axis_to_input_code(SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 0), NAME(FORWARD_AXIS), P1);
    fog_input_add(fog_axis_to_input_code(SDL_CONTROLLER_AXIS_TRIGGERLEFT, 0), NAME(BACKWARD_AXIS), P1);
    fog_input_add(fog_button_to_input_code(SDL_CONTROLLER_BUTTON_A, 0), NAME(DRIFT), P1);
    fog_input_add(fog_axis_to_input_code(SDL_CONTROLLER_AXIS_LEFTX, 1), NAME(LEFTRIGHT), P2);
    fog_input_add(fog_axis_to_input_code(SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 1), NAME(FORWARD_AXIS), P2);
    fog_input_add(fog_axis_to_input_code(SDL_CONTROLLER_AXIS_TRIGGERLEFT, 1), NAME(BACKWARD_AXIS), P2);
    fog_input_add(fog_button_to_input_code(SDL_CONTROLLER_BUTTON_A, 1), NAME(DRIFT), P2);

    car_sprite = fog_asset_fetch_id("CAR_SPRITE");
    car_shape = fog_physics_add_shape_from_sprite(car_sprite);
    square = car_shape;

    car1 = create_car(P1);
    car1.controller = 1;
    car2 = create_car(P2);
    car2.controller = 0;

    char str[100] = {};
    for (u32 i = 0; i < NUM_CAR_SPRITES; i++) {
        sprintf(str, "CAR_RED%d", i);
        car1.sprites[i] = fog_asset_fetch_id(str);
        sprintf(str, "CAR_BLUE%d", i);
        car2.sprites[i] = fog_asset_fetch_id(str);
    }

    for (u32 i = 0; i < NUM_PINE_SPRITES; i++) {
        sprintf(str, "PINE%d", i);
        PINE_SPRITES[i] = fog_asset_fetch_id(str);
    }

    fog_renderer_set_window_size(1200, 800);
    fog_renderer_turn_on_camera(0);
    fog_renderer_turn_on_camera(1);
    fog_renderer_fetch_camera(0)->zoom = 1.0 / 5.0;
    fog_renderer_fetch_camera(1)->zoom = 1.0 / 5.0;

    fog_run(update, draw);

    return 0;
}
