#pragma once


typedef enum {
    LEFT,
    RIGHT,
    FORWARD,
    BACKWARD,
    DRIFT,
    NUM_BINDINGS,
} Binding;
Name bindings[NUM_BINDINGS];

#define NUM_CAR_SPRTIES 16
extern AssetID CAR_SPRITES[NUM_CAR_SPRTIES];

#define NUM_PINE_SPRITES 3
extern AssetID PINE_SPRITES[NUM_PINE_SPRITES];

AssetID fetch_car_sprite(f32 angle);

extern u32 num_bodies;
extern Body *bodies;

#define NAME(binding) bindings[binding]
