#pragma once


typedef enum {
    LEFT,
    RIGHT,
    LEFTRIGHT,
    FORWARD,
    BACKWARD,
    FORWARD_AXIS,
    BACKWARD_AXIS,
    DRIFT,
    PAUSE,
    NUM_BINDINGS,
} Binding;
Name bindings[NUM_BINDINGS];

#define NUM_PINE_SPRITES 3
extern AssetID PINE_SPRITES[NUM_PINE_SPRITES];

extern u32 num_bodies;
extern Body *bodies;

#define NAME(binding) bindings[binding]
