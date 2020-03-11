#include "car.h"

Car create_car(Player player) {
    Car car = {
        .player = player,
        .body = fog_physics_create_body(car_shape, 1.0),

        .acceleration = 2,
        .wheel_rotation_speed = 2,

        .air_resistance = 1,
    };
    car.body.scale = fog_V2(0.1, 0.1);
    return car;
}

void rotate_body(Body *body, f32 angle) {
    body->rotation += angle;
    body->velocity = fog_rotate_v2(body->velocity, angle);
    body->acceleration = fog_rotate_v2(body->acceleration, angle);
    body->force = fog_rotate_v2(body->force, angle);
}

void update_car(Car *car, f32 delta) {

    s32 reversing = fog_rotate_v2(car->body.velocity, -car->body.rotation).y < 0 ? -1 : 1;

    if (fog_input_down(NAME(LEFT), car->player)) {
        rotate_body(&car->body, car->wheel_rotation_speed * reversing * delta);
    }
    if (fog_input_down(NAME(RIGHT), car->player)) {
        rotate_body(&car->body, -car->wheel_rotation_speed * reversing * delta);
    }

    if (fog_input_down(NAME(FORWARD), car->player)) {
        car->body.acceleration = fog_rotate_v2(fog_V2(0, car->acceleration), car->body.rotation);
    } else if (fog_input_down(NAME(BACKWARD), car->player)) {
        car->body.acceleration = fog_rotate_v2(fog_V2(0, -car->acceleration), car->body.rotation);
    } else {
        car->body.acceleration = fog_V2(0, 0);
    }

    car->body.acceleration = fog_add_v2(car->body.acceleration,
                                        fog_mul_v2(car->body.velocity, -car->air_resistance));

    fog_physics_integrate(&car->body, delta);
    for (u32 i = 0; i < num_bodies; i++) {
        fog_physics_solve(fog_physics_check_overlap(&car->body, &bodies[i]));
    }

    #define car_debug_vec(v, c) fog_renderer_push_line(1, car->body.position, fog_add_v2(car->body.position, v), c, 0.01)
    
    car_debug_vec(car->body.velocity, fog_V4(1, 0, 0, 1));
    car_debug_vec(car->body.acceleration, fog_V4(0, 1, 0, 1));
    car_debug_vec(fog_rotate_v2(car->body.velocity, -car->body.rotation), fog_V4(1, 0, 1, 1));
}

void draw_car(Car *car) {
    fog_renderer_push_sprite(0, car_sprite, car->body.position, car->body.scale, car->body.rotation, fog_V4(1, 1, 1, 1));
}

