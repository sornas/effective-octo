#include <fog.h>
#include "game.h"
ShapeID car_shape;
AssetID car_sprite;

#define NUM_CAR_SPRITES 16
#define TIME_REPORT_PREFIX_LENGTH 16

AssetID fetch_car_sprite(AssetID *sprites, f32 angle);

typedef struct Car {
    Player player;
    Body body;
    b8 controller;

    ParticleSystem exhaust_particles;
    ParticleSystem drift_particles;

    ParticleSystem skidmark_particles[4];

    f32 exhaust_spawn_prob;
    f32 drift_spawn_prob;

    f32 wheel_turn_max;
    f32 wheel_turn_speed;
    f32 wheel_turn;

    f32 acceleration;

    f32 wheel_friction_static;

    u32 next_checkpoint;
    u32 current_lap;
    f32 checkpoint_timer;
    f32 prev_checkpoint_time;
    f32 *checkpoint_records;

    char *time_report_prefix;

    AssetID sprites[NUM_CAR_SPRITES];
} Car;


///
// Create a new car.
Car create_car(Player player);

struct Level;
///
// Update the car one step in the time.
void update_car(Car *car, struct Level *lvl, f32 delta);

///
// Draw the car.
void draw_car(Car *car);

////
//
void passed_checkpoint(Car *car, u32 checkpoint);

