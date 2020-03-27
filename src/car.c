#include "car.h"
#include "level.h"
#include <math.h>
#include <stdio.h>

#include "particles.h"

AssetID CAR_SPRITES[NUM_CAR_SPRITES] = {};

#include <stdio.h>

#define PI 3.1416

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

static inline
s32 mod_s32(s32 a, s32 b) {
    return ((a % b) + b) % b;
}

static inline
s32 angle_to_steps(f32 angle, u32 steps) {
    f32 spacing = 2 * 3.1415 / steps;
    return (s32) round(angle / spacing);
}

static inline
f32 snap_rotation(f32 angle, u32 steps) {
    f32 spacing = 2 * 3.1415 / steps;
    return angle_to_steps(angle, steps) * spacing;
}

AssetID fetch_car_sprite(AssetID *sprites, f32 angle) {
    s32 index = angle_to_steps(angle, NUM_CAR_SPRITES);
    return sprites[mod_s32(index + NUM_CAR_SPRITES / 2, NUM_CAR_SPRITES)];
}


Car create_car(Player player) {
    Car car = {
        .player = player,
        .body = fog_physics_create_body(car_shape, 1.0, 0.0, 0.0),

        .exhaust_particles = create_exhaust_particles(),
        .drift_particles = create_drift_particles(),

        .exhaust_spawn_prob = 0.2,
        .drift_spawn_prob = 0.8,

        .wheel_turn_max = PI / 4,
        .wheel_turn_speed = 2,
        .wheel_turn = 0,

        .acceleration = 3,

        .wheel_friction_static = 1,

        .next_checkpoint = 1, // So we don't go into the goal on the first lap.
        .current_lap = 0,
    };

    for (u8 i = 0; i < 4; i++)
        car.skidmark_particles[i] = create_skidmark_particles();

    car.body.scale = fog_V2(0.5, 0.5);
    car.body.damping = 0.5;

    return car;
}

void update_car(Car *car, struct Level *lvl, f32 delta) {
    car->wheel_turn = clamp_f32(-car->wheel_turn_max, car->wheel_turn_max,
            car->wheel_turn + (car->wheel_turn_speed * delta * -fog_input_value(NAME(LEFTRIGHT), car->player)));
    /*
    if (fog_input_down(NAME(LEFT), car->player)) {
        car->wheel_turn = min_f32(car->wheel_turn + (car->wheel_turn_speed * delta),
                                  car->wheel_turn_max);
    } else if (fog_input_down(NAME(RIGHT), car->player)) {
        car->wheel_turn = max_f32(car->wheel_turn - (car->wheel_turn_speed * delta),
                                  -car->wheel_turn_max);
    } else if (fog_input_using_controller()) {
        f32 max = car->wheel_turn_speed * delta;
        if (abs_f32(car->wheel_turn) < max)
            car->wheel_turn = 0;
        else
            car->wheel_turn -= sign_f32(car->wheel_turn) * max;
    }
    */

    if (fog_input_down(NAME(DRIFT), car->player)) {
        car->drift_particles.velocity_dir = (Span) { car->body.rotation + PI - PI/6, car->body.rotation + PI + PI/6 };
        car->drift_particles.position = fog_add_v2(car->body.position, fog_rotate_v2(fog_V2(-0.22, -0.15), car->body.rotation));
        fog_renderer_particle_spawn(&car->drift_particles, 1);
        car->drift_particles.position = fog_add_v2(car->body.position, fog_rotate_v2(fog_V2(-0.22,  0.15), car->body.rotation));
        fog_renderer_particle_spawn(&car->drift_particles, 1);

        //f32 rotation = snap_rotation(car->body.rotation, NUM_CAR_SPRITES);
        f32 rotation = car->body.rotation;
        // front left
        car->skidmark_particles[0].position =
            fog_add_v2(car->body.position,
                    fog_rotate_v2(fog_V2(0.25, 0.15), rotation));
        // front right
        car->skidmark_particles[1].position =
            fog_add_v2(car->body.position,
                    fog_rotate_v2(fog_V2(0.25, -0.15), rotation));
        // back right
        car->skidmark_particles[2].position =
            fog_add_v2(car->body.position,
                    fog_rotate_v2(fog_V2(-0.25, -0.15), rotation));
        // back left
        car->skidmark_particles[3].position =
            fog_add_v2(car->body.position,
                    fog_rotate_v2(fog_V2(-0.25, 0.15), rotation));
        
        for (u8 i = 0; i < 4; i++) {
            car->skidmark_particles[i].rotation = (Span) { rotation, rotation };
            fog_renderer_particle_spawn(&car->skidmark_particles[i], 1);
        }
    }

    Vec2 forward = fog_V2(cos(car->body.rotation), sin(car->body.rotation));
    Vec2 acceleration = fog_V2(0, 0);
    f32 dacc = car->acceleration;
    f32 forward_backward = fog_input_value(NAME(FORWARD_AXIS), car->player)
                         - fog_input_value(NAME(BACKWARD_AXIS), car->player);
    acceleration = fog_mul_v2(forward, dacc * forward_backward);
    car->exhaust_particles.velocity_dir = (Span) { car->body.rotation + PI, car->body.rotation + PI };
    //fog_renderer_particle_spawn(&car->exhaust_particles, 1);  TODO(gu)
    if (fog_input_down(NAME(FORWARD), car->player)) {
        acceleration = fog_mul_v2(forward, dacc);
        car->exhaust_particles.velocity_dir = (Span) { car->body.rotation + PI, car->body.rotation + PI };
        fog_renderer_particle_spawn(&car->exhaust_particles, 1);
    } else if (fog_input_down(NAME(BACKWARD), car->player)) {
        acceleration = fog_mul_v2(forward, -dacc);
    }
    Vec2 vel = fog_add_v2(car->body.velocity, fog_mul_v2(acceleration, delta));
    f32 turn = car->wheel_turn * delta * fog_dot_v2(forward, vel);
    f32 rotation = car->body.rotation + turn;

    const f32 MAX_ROTATION = 2.0;
    f32 rot_mag = min_f32(fog_dot_v2(forward, vel), MAX_ROTATION);
    vel = fog_sub_v2(vel, fog_mul_v2(forward, rot_mag));
    Vec2 new_forward = fog_V2(cos(rotation), sin(rotation));
    vel = fog_add_v2(vel, fog_mul_v2(new_forward, rot_mag));

    car->body.rotation = rotation;
    car->body.velocity = vel;
    // Collisions
    fog_physics_integrate(&car->body, delta);
    for (u32 i = 0; i < lvl->num_bodies; i++) {
        Body *body = lvl->bodies + i;
        Overlap overlap = fog_physics_check_overlap(&car->body, body);
        fog_physics_solve(overlap);
    }

    // Update checkpoints
    {
        Vec2 cp = lvl->checkpoints[car->next_checkpoint];
        Vec2 norm = fog_sub_v2(car->body.position, cp);
        Vec2 d = lvl->checkpoints_dir[car->next_checkpoint];
        b8 passed = fog_dot_v2(norm, d) > 0;
        b8 forward = fog_dot_v2(car->body.velocity, d) > 0;
        if (passed && forward) {
            if (car->next_checkpoint == 0) {
                car->current_lap++;
            }
            car->next_checkpoint++;
            car->next_checkpoint %= lvl->num_checkpoints;
        }
    }

    car->exhaust_particles.position = car->body.position;
    car->drift_particles.position = car->body.position;
    for (u8 i = 0; i < 4; i++)
        fog_renderer_particle_update(&car->skidmark_particles[i], delta);
    fog_renderer_particle_update(&car->exhaust_particles, delta);
    fog_renderer_particle_update(&car->drift_particles, delta);

#define car_debug_vec(v, o, c)                                                \
    fog_renderer_push_line(                                                   \
        1, fog_add_v2(car->body.position, o),                                 \
        fog_add_v2(fog_add_v2(car->body.position, fog_mul_v2(v, 0.2)), o), c, \
        0.02)
#define world_debug_vec(v, o, c)                                              \
    fog_renderer_push_line(1, o, fog_add_v2(o, v), c, 0.02)

    //car_debug_vec(car_dir, fog_V2(0, 0), fog_V4(0, 1, 0, 1));
    //car_debug_vec(fric_total, fog_V2(0, 0), fog_V4(1, 0, 0, 1));

    //Vec2 f = fog_mul_v2(car_dir, car_length);
    //car_debug_vec(front_normal, f, fog_V4(1, 0, 0, 1));
    //car_debug_vec(fog_neg_v2(front_normal), f, fog_V4(1, 0, 0, 1));
    //car_debug_vec(front_fric, f, fog_V4(1, 0, 1, 1));

    //Vec2 b = fog_mul_v2(fog_neg_v2(car_dir), car_length);
    //car_debug_vec(back_normal, b, fog_V4(1, 0, 0, 1));
    //car_debug_vec(fog_neg_v2(back_normal), b, fog_V4(1, 0, 0, 1));
    //car_debug_vec(back_fric, b, fog_V4(1, 1, 0, 1));
    //
    //world_debug_vec(car->exhaust_particles.position, fog_V2(0, 0), fog_V4(0, 0, 0, 1));
}

void draw_car(Car *car) {
    fog_physics_debug_draw_body(&car->body);
    AssetID sprite = fetch_car_sprite(car->sprites, car->body.rotation);
    fog_renderer_push_sprite(2, sprite, car->body.position, fog_mul_v2(car->body.scale, 5), 0, fog_V4(1, 1, 1, 1));

    for (u8 i = 0; i < 4; i++)
        fog_renderer_particle_draw(&car->skidmark_particles[i]);
    fog_renderer_particle_draw(&car->exhaust_particles);
    fog_renderer_particle_draw(&car->drift_particles);
}

