#include <fog.h>
#include "game.h"
ShapeID car_shape;
AssetID car_sprite;

#define NUM_CAR_SPRITES 16
extern AssetID CAR_SPRITES[NUM_CAR_SPRITES];

AssetID fetch_car_sprite(f32 angle);

typedef struct Car {
    Player player;
    Body body;

    ParticleSystem exhaust_particles;

    f32 wheel_turn_max;
    f32 wheel_turn_speed;
    f32 wheel_turn;

    f32 acceleration;

    f32 wheel_friction_static;

    u32 next_checkpoint;
    u32 current_lap;
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

