#include <fog.h>
#include "game.h"
ShapeID car_shape;
AssetID car_sprite;

typedef struct {
    Player player;
    Body body;

    s32 reversing;  // 1 <=> forwards, -1 <=> backwards, standing still UB

    f32 acceleration;
    f32 wheel_rotation_max;
    f32 wheel_rotation_current;

    f32 max_velocity;
    f32 brake_air;

    f32 drift_turn_modifier;
} Car;

///
// Craete a new car.
Car create_car(Player player);

///
// Update the car one step in the time.
void update_car(Car *car, f32 delta);

///
// Draw the car.
void draw_car(Car *car);

