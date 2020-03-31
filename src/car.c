#include "car.h"
#include "level.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

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
f32 lerp_f32(f32 a, f32 b, f32 l) {
    return a * (1.0f - l) + (b * l);
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
        .wheel_turn = 0,

        .acceleration = 3,

        .wheel_friction_static = 1,

        .next_checkpoint = 1, // So we don't go into the goal on the first lap.
        .current_lap = 0,
        .checkpoint_timer = 0,
        .prev_checkpoint_time = 0,
        .checkpoint_records = NULL,

        .time_report = {},
    };

    for (u8 i = 0; i < 4; i++)
        car.skidmark_particles[i] = create_skidmark_particles();

    car.body.scale = fog_V2(0.5, 0.3);
    car.body.damping = 0.5;

    return car;
}

void update_car(Car *car, struct Level *lvl, f32 delta) {
    const f32 DRIFT_THRESH = 0.35;
    const f32 DRIFT_SPEED = 0.85;
    car->checkpoint_timer += delta;

    Vec2 forward = fog_V2(cos(car->body.rotation), sin(car->body.rotation));
    bool drifting = false;
    if (abs_f32(fog_dot_v2(fog_rotate_ccw_v2(forward),
                   fog_normalize_v2(car->body.velocity))) > DRIFT_THRESH
        && fog_length_v2(car->body.velocity) > DRIFT_SPEED) {
        drifting = true;
        car->drift_particles.velocity_dir = (Span) { car->body.rotation + PI - PI/6, car->body.rotation + PI + PI/6 };
        car->drift_particles.position = fog_add_v2(car->body.position, fog_rotate_v2(fog_V2(-0.22, -0.15), car->body.rotation));
        fog_renderer_particle_spawn(&car->drift_particles, 1);
        car->drift_particles.position = fog_add_v2(car->body.position, fog_rotate_v2(fog_V2(-0.22,  0.15), car->body.rotation));
        fog_renderer_particle_spawn(&car->drift_particles, 1);

        //f32 rotation = snap_rotation(car->body.rotation, NUM_CAR_SPRITES);
        f32 rotation = car->body.rotation;
        // front left
        car->skidmark_particles[0].position = fog_add_v2(
            car->body.position, fog_rotate_v2(fog_V2(0.25, 0.15), rotation));
        // front right
        car->skidmark_particles[1].position = fog_add_v2(
            car->body.position, fog_rotate_v2(fog_V2(0.25, -0.15), rotation));
        // back right
        car->skidmark_particles[2].position = fog_add_v2(
            car->body.position, fog_rotate_v2(fog_V2(-0.25, -0.15), rotation));
        // back left
        car->skidmark_particles[3].position = fog_add_v2(
            car->body.position, fog_rotate_v2(fog_V2(-0.25, 0.15), rotation));

        for (u8 i = 0; i < 4; i++) {
            car->skidmark_particles[i].rotation = (Span) { rotation, rotation };
            fog_renderer_particle_spawn(&car->skidmark_particles[i], 1);
        }
    }


    f32 WHEEL_TURN_SPEED;
    f32 MAX_ROTATION;
    if (drifting) {
        MAX_ROTATION = 1.0;
        car->body.damping = 0.5;
        WHEEL_TURN_SPEED = 0.8;
    } else {
        WHEEL_TURN_SPEED = 2.0,
        MAX_ROTATION = 2.0;
        car->body.damping = 0.4;
    }

    if (car->controller) {
        f32 wheel_target = car->wheel_turn_max * -fog_input_value(NAME(LEFTRIGHT), car->player);
        car->wheel_turn = lerp_f32(car->wheel_turn, wheel_target, WHEEL_TURN_SPEED * delta * 2.5);
    } else {
        if (fog_input_down(NAME(LEFT), car->player)) {
            car->wheel_turn = min_f32(car->wheel_turn + (WHEEL_TURN_SPEED * delta),
                                      car->wheel_turn_max);
        } else if (fog_input_down(NAME(RIGHT), car->player)) {
            car->wheel_turn = max_f32(car->wheel_turn - (WHEEL_TURN_SPEED * delta),
                                      -car->wheel_turn_max);
        } else {
            f32 max = WHEEL_TURN_SPEED * delta;
            if (abs_f32(car->wheel_turn) < max)
                car->wheel_turn = 0;
            else
                car->wheel_turn -= sign_f32(car->wheel_turn) * max;
        }
    }

    Vec2 acceleration = fog_V2(0, 0);
    f32 dacc = car->acceleration;
    if (car->controller) {
        f32 forward_backward = fog_input_value(NAME(FORWARD_AXIS), car->player)
                             - fog_input_value(NAME(BACKWARD_AXIS), car->player);
        acceleration = fog_mul_v2(forward, dacc * forward_backward);
        car->exhaust_spawn_prob = lerp_f32(0.2, 0.6, fog_input_value(NAME(FORWARD_AXIS), car->player));
        //fog_renderer_particle_spawn(&car->exhaust_particles, 1);  TODO(gu)
    } else {
        if (fog_input_down(NAME(FORWARD), car->player)) {
            acceleration = fog_mul_v2(forward, dacc);
            car->exhaust_spawn_prob = 0.6;
        } else if (fog_input_down(NAME(BACKWARD), car->player)) {
            acceleration = fog_mul_v2(forward, -dacc);
            car->exhaust_spawn_prob = 0.2;
        } else {
            car->exhaust_spawn_prob = 0.2;
        }
    }
    Vec2 vel = fog_add_v2(car->body.velocity, fog_mul_v2(acceleration, delta));
    f32 turn = car->wheel_turn * delta * fog_dot_v2(forward, vel);
    f32 rotation = car->body.rotation + turn;

    car->exhaust_particles.velocity_dir = (Span) { car->body.rotation + PI - 0.4, car->body.rotation + PI + 0.4 };
    if (fog_random_real(0, 1) < car->exhaust_spawn_prob)
        fog_renderer_particle_spawn(&car->exhaust_particles, 1);


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
        if (overlap.is_valid) {
            car->body.velocity = fog_mul_v2(car->body.velocity, pow(0.1, delta));
        }

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
            car->prev_checkpoint_time = car->checkpoint_timer;
            if (car->next_checkpoint == 0) {
                car->current_lap++;
                passed_checkpoint(car, lvl->num_checkpoints);
            } else {
                // this is easier than working with negative mod
                passed_checkpoint(car, car->next_checkpoint - 1);
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

    sprintf(car->time_report, " %d (%d): %.2f",
            (car->player == P1 ? 1 : 2),
            (car->next_checkpoint + lvl->num_checkpoints - (car->checkpoint_timer < 0.5 ? 2 : 1)) % lvl->num_checkpoints,
            (car->checkpoint_timer < 0.5 ? car->prev_checkpoint_time : car->checkpoint_timer));
    fog_util_tweak_show(car->time_report);

#define car_debug_vec(v, o, c)                                                \
    fog_renderer_push_line(                                                   \
        1, fog_add_v2(car->body.position, o),                                 \
        fog_add_v2(fog_add_v2(car->body.position, fog_mul_v2(v, 0.2)), o), c, \
        0.02)
#define world_debug_vec(v, o, c)                                              \
    fog_renderer_push_line(1, o, fog_add_v2(o, v), c, 0.02)


}

void collision_car(Car *a, Car *b) {
    fog_physics_solve(fog_physics_check_overlap(&a->body, &b->body));
}

void draw_car(Car *car) {
    AssetID sprite = fetch_car_sprite(car->sprites, car->body.rotation);
    fog_renderer_push_sprite(2, sprite, car->body.position, fog_mul_v2(fog_V2(1, 1), 3), 0, fog_V4(1, 1, 1, 1));

    for (u8 i = 0; i < 4; i++)
        fog_renderer_particle_draw(&car->skidmark_particles[i]);
    fog_renderer_particle_draw(&car->exhaust_particles);
    fog_renderer_particle_draw(&car->drift_particles);
}

void passed_checkpoint(Car *car, u32 checkpoint) {
    if (car->checkpoint_records[checkpoint] < 0.1
            || car->checkpoint_timer < car->checkpoint_records[checkpoint]) {
        car->checkpoint_records[checkpoint] = car->checkpoint_timer;
    }
    car->checkpoint_timer = 0;
}

void reset_car(Car *car, u32 num_checkpoints) {
    car->next_checkpoint = 1;
    car->current_lap = 0;
    car->checkpoint_timer = 0;
    car->checkpoint_records = realloc(car->checkpoint_records, num_checkpoints * sizeof(f32));
    car->body.velocity = fog_V2(0, 0);
}
