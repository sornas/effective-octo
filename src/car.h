#include <fog.h>
#include "game.h"
ShapeID car_shape;
AssetID car_sprite;

typedef struct {
    Player player;
    Body body;
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
