#include <fog.h>
#include "game.h"
ShapeID car_shape;
AssetID car_sprite;

#define NUM_CAR_SPRTIES 16
extern AssetID CAR_SPRITES[NUM_CAR_SPRTIES];

AssetID fetch_car_sprite(f32 angle);

typedef struct {
    Player player;
    Body body;

    ParticleSystem exhaust_particles;

    f32 wheel_turn_max;
    f32 wheel_turn_speed;
    f32 wheel_turn;

    f32 acceleration;

    f32 wheel_friction_static;
} Car;

///
// Create a new car.
Car create_car(Player player);

///
// Update the car one step in the time.
void update_car(Car *car, f32 delta);

///
// Draw the car.
void draw_car(Car *car);

