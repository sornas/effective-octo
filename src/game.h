#pragma once


typedef enum {
    LEFT,
    RIGHT,
    FORWARD,
    BACKWARD,
    BRAKE,
    NUM_BINDINGS,
} Binding;
Name bindings[NUM_BINDINGS];

extern u32 num_bodies;
extern Body *bodies;

#define NAME(binding) bindings[binding]
