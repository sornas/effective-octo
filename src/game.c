#include <stdio.h>
#include <stdlib.h>
#include <fog.h>

typedef enum {
    LEFT,
    RIGHT,
    FORWARD,
    BACKWARD,
    NUM_BINDINGS,
} Binding;
Name bindings[NUM_BINDINGS];

#define NAME(binding) bindings[binding]

ShapeID car_shape;
u32 num_bodies = 3;
Body *bodies;

typedef struct {
    Player player;
    Body body;
} Car;

Car create_car(Player player) {
    Car car = {
        .player = player,
        .body = fog_physics_create_body(car_shape, 1.0),
    };
    car.body.scale = fog_V2(0.1, 0.1);
    return car;
}

Car car;
AssetID car_sprite;

void update_car(Car *car, f32 delta) {
    Vec2 d_pos = fog_V2(0, 0);
    if (fog_input_down(NAME(LEFT), car->player)) {
        d_pos.x += -1;
    }
    if (fog_input_down(NAME(RIGHT), car->player)) {
        d_pos.x += 1;
    }
    if (fog_input_down(NAME(FORWARD), car->player)) {
        d_pos.y += 1;
    }
    if (fog_input_down(NAME(BACKWARD), car->player)) {
        d_pos.y += -1;
    }
    car->body.position = fog_add_v2(car->body.position, fog_mul_v2(d_pos, delta * 0.5));
    for (u32 i = 0; i < num_bodies; i++) {
        fog_physics_solve(fog_physics_check_overlap(&car->body, &bodies[i]));
    }
}

void draw_car(Car *car) {
    fog_renderer_push_sprite(0, car_sprite, car->body.position, car->body.scale, 0, fog_V4(1, 1, 1, 1));
}

void update() {
    update_car(&car, fog_logic_delta());
}

void draw() {
    draw_car(&car);
    for (u32 i = 0; i < num_bodies; i++) {
        fog_physics_debug_draw_body(&bodies[i]);
    }
}

int main(int argc, char **argv) {
    for (u32 i = 0; i < NUM_BINDINGS; i++) {
        bindings[i] = fog_input_request_name(1);
    }

    fog_init(argc, argv);

    fog_input_add(fog_key_to_input_code(SDLK_a), NAME(LEFT), P1);
    fog_input_add(fog_key_to_input_code(SDLK_d), NAME(RIGHT), P1);
    fog_input_add(fog_key_to_input_code(SDLK_w), NAME(FORWARD), P1);
    fog_input_add(fog_key_to_input_code(SDLK_s), NAME(BACKWARD), P1);

    car_sprite = fog_asset_fetch_id("CAR_SPRITE");
    car_shape = fog_physics_add_shape_from_sprite(car_sprite);
    car = create_car(P1);
    //body = fog_physics_create_body(car_shape, 0);

    bodies = malloc(sizeof(Body) * num_bodies);
    for (u32 i = 0; i < num_bodies; i++) {
        bodies[i] = fog_physics_create_body(car_shape, 0);
        bodies[i].position = fog_random_unit_vec2();
        bodies[i].scale = fog_random_unit_vec2();
    }

    fog_run(update, draw);

    return 0;
}
