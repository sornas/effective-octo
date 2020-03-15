#include "car.h"
#include <math.h>

#define PI 3.1416

///*
// Set a vector to {0, 0} in-place.
void reset_vec2(Vec2 *v) {
    v->x = 0;
    v->y = 0;
}

static inline
f32 min_f32(f32 a, f32 b) {
    return a < b ? a : b;
}

static inline
f32 max_f32 (f32 a, f32 b) {
    return a > b ? a : b;
}

static inline
f32 cross_v2(Vec2 a, Vec2 b) {
    return a.y * b.x - a.x * b.y;
}

static inline
f32 clamp_f32(f32 min, f32 max, f32 v) {
    return min_f32(max, max_f32(min, v));
}

static inline
f32 sign_f32(f32 a) {
    if (a >= 0)
        return 1;
    return -1;
}

static inline
f32 abs_f32(f32 a) {
    if (a >= 0)
        return a;
    return -a;
}

Car create_car(Player player) {
    Car car = {
        .player = player,
        .body = fog_physics_create_body(car_shape, 1.0, 0.0, 0.0),

        .wheel_turn_max = PI / 4,
        .wheel_turn_speed = 2,
        .wheel_turn = 0,

        .acceleration = 3,

        .wheel_friction_static = 1,
    };
    car.body.scale = fog_V2(0.1, 0.1);
    car.body.damping = 0.3;
    return car;
}

void update_car(Car *car, f32 delta) {
    if (fog_input_down(NAME(FORWARD), car->player)) {
        car->body.acceleration = fog_V2(car->acceleration * cos(car->body.rotation),
                                        car->acceleration * sin(car->body.rotation));
    } else if (fog_input_down(NAME(BACKWARD), car->player)) {
        car->body.acceleration = fog_V2(-car->acceleration * cos(car->body.rotation),
                                        -car->acceleration * sin(car->body.rotation));
    } else {
        car->body.acceleration = fog_V2(0, 0);
    }

    if (fog_input_down(NAME(LEFT), car->player)) {
        car->wheel_turn = min_f32(car->wheel_turn + (car->wheel_turn_speed * delta),
                                  car->wheel_turn_max);
    } else if (fog_input_down(NAME(RIGHT), car->player)) {
        car->wheel_turn = max_f32(car->wheel_turn - (car->wheel_turn_speed * delta),
                                  -car->wheel_turn_max);
    } else {
        f32 max = car->wheel_turn_speed * delta;
        if (abs_f32(car->wheel_turn) < max)
            car->wheel_turn = 0;
        else
            car->wheel_turn -= sign_f32(car->wheel_turn) * max;
    }

    static f32 car_length = 0.5;
    static f32 max_friction_front = 2.0;
    static f32 grip_constant_front = 1.0;

    static f32 max_friction_back = 1.5;
    static f32 grip_constant_back = 1.0;

    fog_util_tweak_f32("car_length", &car_length, 0.1);

    fog_util_tweak_f32("max front", &max_friction_front, 0.1);
    fog_util_tweak_f32("grip front", &grip_constant_front, 0.1);

    grip_constant_back = min_f32(2.0 / (fog_length_v2(car->body.velocity) + 0.1), 1.0);

    fog_util_tweak_f32("max back", &max_friction_back, 0.1);
    fog_util_tweak_f32("grip back", &grip_constant_back, 0.1);

    f32 mu = car->wheel_friction_static;
    f32 rot = car->body.rotation;
    Vec2 i_hat = fog_V2(1.0, 0);

    Vec2 dvel = fog_mul_v2(car->body.acceleration, delta);
    Vec2 vel = fog_add_v2(car->body.velocity, dvel);

    Vec2 fric_total = fog_mul_v2(vel, -mu);
    Vec2 car_dir = fog_rotate_v2(i_hat, rot);

    f32 angular_velocity = 0;

    Vec2 front_normal = fog_rotate_v2(i_hat, rot + car->wheel_turn - PI / 2);
    f32  front_fric_scale = clamp_f32(-max_friction_front, max_friction_front,
                                  fog_dot_v2(fric_total, front_normal) * grip_constant_front);
    Vec2 front_fric = fog_mul_v2(front_normal, front_fric_scale);
    angular_velocity += cross_v2(front_fric, car_dir) / (car_length * grip_constant_front);

    Vec2 back_normal = fog_rotate_v2(i_hat, rot - PI / 2);
    f32  back_fric_scaler = clamp_f32(-max_friction_back, max_friction_back,
                                       fog_dot_v2(fric_total, back_normal) * grip_constant_back);
    Vec2 back_fric = fog_mul_v2(back_normal, back_fric_scaler);
    angular_velocity -= cross_v2(back_fric, car_dir) / (car_length * grip_constant_back);
    car->body.rotation += angular_velocity * delta;

    Vec2 friction = fog_V2(0, 0);
    friction = fog_add_v2(friction, front_fric);
    friction = fog_add_v2(friction, back_fric);

    Vec2 new_forward = fog_rotate_v2(i_hat, car->body.rotation);
    Vec2 vel_comp = fog_mul_v2(new_forward, -0.5 * fog_dot_v2(friction, new_forward));
    car->body.acceleration = fog_add_v2(car->body.acceleration, vel_comp);

    car->body.acceleration = fog_add_v2(car->body.acceleration, friction);

    fog_physics_integrate(&car->body, delta);
    for (u32 i = 0; i < num_bodies; i++) {
        fog_physics_solve(fog_physics_check_overlap(&car->body, &bodies[i]));
    }

#define car_debug_vec(v, o, c)                                                \
    fog_renderer_push_line(                                                   \
        1, fog_add_v2(car->body.position, o),                                 \
        fog_add_v2(fog_add_v2(car->body.position, fog_mul_v2(v, 0.2)), o), c, \
        0.02)

    car_debug_vec(car_dir, fog_V2(0, 0), fog_V4(0, 1, 0, 1));
    car_debug_vec(fric_total, fog_V2(0, 0), fog_V4(1, 0, 0, 1));


    Vec2 f = fog_mul_v2(car_dir, car_length);
    car_debug_vec(front_normal, f, fog_V4(1, 0, 0, 1));
    car_debug_vec(fog_neg_v2(front_normal), f, fog_V4(1, 0, 0, 1));
    car_debug_vec(front_fric, f, fog_V4(1, 0, 1, 1));

    Vec2 b = fog_mul_v2(fog_neg_v2(car_dir), car_length);
    car_debug_vec(back_normal, b, fog_V4(1, 0, 0, 1));
    car_debug_vec(fog_neg_v2(back_normal), b, fog_V4(1, 0, 0, 1));
    car_debug_vec(back_fric, b, fog_V4(1, 1, 0, 1));
}

void draw_car(Car *car) {
    fog_renderer_push_sprite(0, car_sprite, car->body.position, car->body.scale, 0, fog_V4(1, 1, 1, 1));
}

