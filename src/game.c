#include <stdio.h>
#include <stdlib.h>
#define FOG_IMPL
#include <fog.h>

#include "game.h"
#include "car.h"

u32 num_bodies = 4;
Body *bodies;
Car car;

void update() {
    update_car(&car, fog_logic_delta());
}

void draw() {
    draw_car(&car);
    for (u32 i = 0; i < num_bodies; i++) {
        fog_physics_debug_draw_body(&bodies[i]);
    }
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
    car = create_car(P1);
    //body = fog_physics_create_body(car_shape, 0);

    bodies = malloc(sizeof(Body) * num_bodies);
    for (u32 i = 0; i < num_bodies; i++) {
        bodies[i] = fog_physics_create_body(car_shape, 0);
        bodies[i].position = fog_random_unit_vec2();
        bodies[i].scale = fog_random_unit_vec2();
    }

    fog_run(update, draw);

    return 0;
}
