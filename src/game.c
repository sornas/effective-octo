#include <stdio.h>
#include <stdlib.h>
#define FOG_IMPL
#include <fog.h>

#include "game.h"
#include "car.h"

u32 num_bodies = 4;
Body *bodies;
Car car;

AssetID CAR_SPRITES[NUM_CAR_SPRTIES] = {};
AssetID PINE_SPRITES[NUM_PINE_SPRITES] = {};

AssetID fetch_car_sprite(f32 angle) {
    const f32 spacing = 2 * 3.1415 / NUM_CAR_SPRTIES;
    s32 index = (s32) (angle / spacing);
    return CAR_SPRITES[(index + NUM_CAR_SPRTIES / 2) % NUM_CAR_SPRTIES];
}

void update() {
    update_car(&car, fog_logic_delta());
}

void draw() {
    draw_car(&car);
    for (u32 i = 0; i < num_bodies; i++) {
        fog_physics_debug_draw_body(&bodies[i]);
    }
    static f32 angle = 0;
    fog_util_tweak_f32("angle", &angle, 0.1);
    fog_renderer_push_sprite(0, fetch_car_sprite(angle), fog_V2(0, 0), fog_V2(1, 1), 0, fog_V4(1, 1, 1, 1));


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

    car_sprite = fog_asset_fetch_id("CAR_SPRITE");

    char str[100] = {};
    for (u32 i = 0; i < NUM_CAR_SPRTIES; i++) {
        sprintf(str, "CAR%d", i);
        CAR_SPRITES[i] = fog_asset_fetch_id(str);
    }

    for (u32 i = 0; i < NUM_PINE_SPRITES; i++) {
        sprintf(str, "PINE%d", i);
        PINE_SPRITES[i] = fog_asset_fetch_id(str);
    }

    car_shape = fog_physics_add_shape_from_sprite(car_sprite);
    car = create_car(P1);
    //body = fog_physics_create_body(car_shape, 0);

    bodies = malloc(sizeof(Body) * num_bodies);
    for (u32 i = 0; i < num_bodies; i++) {
        bodies[i] = fog_physics_create_body(car_shape, 0, 0.0, 0.0);
        bodies[i].position = fog_random_unit_vec2();
        bodies[i].scale = fog_random_unit_vec2();
    }

    fog_run(update, draw);

    return 0;
}
