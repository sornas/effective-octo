#include "car.h"

Car create_car(Player player) {
    Car car = {
        .player = player,
        .body = fog_physics_create_body(car_shape, 1.0, 0.0, 0.0),
    };
    car.body.scale = fog_V2(0.1, 0.1);
    return car;
}


void update_car(Car *car, f32 delta) {
    Vec2 d_pos = fog_V2(0, 0);
    if (fog_input_down(NAME(LEFT), car->player)) {
        d_pos.x += -1;
    }
    if (fog_input_down(NAME(RIGHT), car->player)) {
        d_pos.x += 1;
    }
    if (fog_input_down(NAME(FORWARD), car->player)) {
        d_pos.y += 1;
    }
    if (fog_input_down(NAME(BACKWARD), car->player)) {
        d_pos.y += -1;
    }
    car->body.position = fog_add_v2(car->body.position, fog_mul_v2(d_pos, delta * 0.5));
    for (u32 i = 0; i < num_bodies; i++) {
        fog_physics_solve(fog_physics_check_overlap(&car->body, &bodies[i]));
    }
}

void draw_car(Car *car) {
    fog_renderer_push_sprite(0, car_sprite, car->body.position, car->body.scale, 0, fog_V4(1, 1, 1, 1));
}

