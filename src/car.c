#include "car.h"
#include <math.h>

#define PI 3.1416

///*
// Set a vector to {0, 0} in-place.
void reset_vec2(Vec2 *v) {
    v->x = 0;
    v->y = 0;
}

f32 min_f32(f32 a, f32 b) {
    return a < b ? a : b;
}

f32 max_f32 (f32 a, f32 b) {
    return a > b ? a : b;
}

Car create_car(Player player) {
    Car car = {
        .player = player,
        .body = fog_physics_create_body(car_shape, 1.0),

        .wheel_turn_max = 1,
        .wheel_turn_speed = 2,
        .wheel_turn = 0,

        .acceleration = 3,
        .brake = 5,

        .wheel_friction_static = 1,
        .wheel_friction_static_max = 3,
        .wheel_friction_dynamic = 0.5,
    };
    car.body.scale = fog_V2(0.1, 0.1);
    return car;
}

void update_car(Car *car, f32 delta) {
    fog_renderer_fetch_camera(0)->zoom = 0.5;
    fog_renderer_set_window_size(1000, 1000);

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
    }
    if (fog_input_down(NAME(RIGHT), car->player)) {
        car->wheel_turn = max_f32(car->wheel_turn - (car->wheel_turn_speed * delta),
                                  -car->wheel_turn_max);
    }
#if 0
    //x1y2 - x2y1
    Vec2 front_pos = fog_rotate_v2(fog_div_v2(fog_V2(car->body.scale.x, 0), 2),
                                   car->body.rotation);
    Vec2 wheel_friction_front = fog_rotate_v2(fog_normalize_v2(front_pos), car->wheel_turn - PI/2);

    Vec2 wheel_friction_front_comp = fog_mul_v2(wheel_friction_front, fog_dot_v2(car->body.velocity, wheel_friction_front) * car->wheel_friction_static);

    f32 angular_velocity = front_pos.x * wheel_friction_front_comp.y - front_pos.y * wheel_friction_front_comp.x;

    car->body.rotation += angular_velocity;
#endif

#if 1
    Vec2 next_velocity = fog_add_v2(car->body.velocity, fog_mul_v2(car->body.acceleration, delta));
    Vec2 wheel_friction_sum = fog_mul_v2(next_velocity, -car->wheel_friction_static);
    Vec2 car_dir = fog_rotate_v2(fog_V2(1.0, 0), car->body.rotation);
    Vec2 turn_normal = fog_rotate_v2(fog_V2(1.0, 0), car->body.rotation + car->wheel_turn - PI / 2);
    Vec2 turn_friction = fog_mul_v2(turn_normal, -fog_dot_v2(wheel_friction_sum, turn_normal));

    Vec2 wheel_normal = fog_rotate_v2(fog_V2(1.0, 0), car->body.rotation - PI / 2);
    Vec2 wheel_friction = fog_mul_v2(wheel_normal, -fog_dot_v2(wheel_friction_sum, wheel_normal));
    f32 angular_velocity = wheel_friction.y * car_dir.x - wheel_friction.x * car_dir.y;


#endif

    fog_util_tweak_f32("wheel turn", &car->wheel_turn, 0.5);
    fog_util_tweak_f32("rotation", &car->body.rotation, -1);
    fog_util_tweak_f32("angular_velocity", &angular_velocity, 1);

    //car->body.velocity = fog_add_v2(car->body.velocity, fog_mul_v2(wheel_friction_front_comp, delta));

    if (fog_input_down(NAME(BRAKE), car->player)) {
    }

    car->body.rotation += angular_velocity;
    // car->body.acceleration = fog_sub_v2(car->body.acceleration, wheel_side_friction); 
    f32 static_mu = min_f32(max_f32(fog_dot_v2(car->body.acceleration, turn_friction), 0), car->wheel_friction_static_max);
    car->body.acceleration = fog_add_v2(car->body.acceleration, fog_mul_v2(wheel_normal, static_mu)); 
    fog_physics_integrate(&car->body, delta);
    for (u32 i = 0; i < num_bodies; i++) {
        fog_physics_solve(fog_physics_check_overlap(&car->body, &bodies[i]));
    }

#define car_debug_vec(v, c) fog_renderer_push_line(1, car->body.position, fog_add_v2(car->body.position, v), c, 0.01)

    car_debug_vec(car_dir, fog_V4(0, 1, 0, 1));
    car_debug_vec(wheel_friction_sum, fog_V4(1, 0, 0, 1));
    car_debug_vec(wheel_normal, fog_V4(1, 1, 0, 1));
    car_debug_vec(fog_neg_v2(wheel_normal), fog_V4(1, 1, 0, 1));
    car_debug_vec(fog_neg_v2(turn_normal), fog_V4(0, 1, 1, 1));
    car_debug_vec(turn_normal, fog_V4(0, 1, 1, 1));
    car_debug_vec(fog_neg_v2(turn_friction), fog_V4(1, 0, 1, 1));

#if 0
    fog_renderer_push_line(1, car->body.position, fog_add_v2(car->body.position, car->body.acceleration), fog_V4(0, 0, 1, 1), 0.01);
    fog_renderer_push_line(1, car->body.position, fog_add_v2(car->body.position, front_pos), fog_V4(1, 0, 0, 1), 0.01);
    //fog_renderer_push_line(1, car->body.position, fog_add_v2(car->body.position, wheel_friction_front), fog_V4(1, 0, 0, 1), 0.01);
    fog_renderer_push_line(2, car->body.position, fog_add_v2(car->body.position, wheel_friction_front), fog_V4(1, 0, 1, 1), 0.01);
    fog_renderer_push_line(2, car->body.position, fog_add_v2(car->body.position, car->body.velocity), fog_V4(1, 0, 1, 1), 0.01);
    fog_renderer_push_line(2, car->body.position, fog_add_v2(car->body.position, wheel_friction_front_comp), fog_V4(0, 1, 0, 1), 0.01);
#endif
    
}

void draw_car(Car *car) {
    fog_renderer_push_sprite(0, car_sprite, car->body.position, car->body.scale, car->body.rotation - (PI/2), fog_V4(1, 1, 1, 1));
}

