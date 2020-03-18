#include <fog.h>

#include "particles.h"

ParticleSystem create_exhaust_particles() {
    ParticleSystem exhaust_particles = fog_renderer_create_particle_system(0, 500, fog_V2(0, 0));
    exhaust_particles.one_color = 1;
    exhaust_particles.one_alpha = 0;
    exhaust_particles.one_size = 0;
    exhaust_particles.alive_time = (Span) { 1, 1 };
    exhaust_particles.position_x = (Span) { 0, 0 };
    exhaust_particles.position_y = (Span) { 0, 0 };
    exhaust_particles.velocity = (Span) { 1, 1 };
    exhaust_particles.acceleration = (Span) { 0, 0 };
    exhaust_particles.spawn_size = (Span) { 0, 0 };
    exhaust_particles.spawn_red = (Span) { 0, 0 };
    exhaust_particles.spawn_green = (Span) { 0, 0 };
    exhaust_particles.spawn_blue = (Span) { 0, 0 };
    exhaust_particles.spawn_alpha = (Span) { 0.8, 1 };
    exhaust_particles.die_size = (Span) { 0.5, 1 };
    exhaust_particles.die_alpha = (Span) { 0, 0 };
    return exhaust_particles;
}

ParticleSystem create_drift_particles() {
    ParticleSystem drift_particles = fog_renderer_create_particle_system(0, 500, fog_V2(0, 0));
    drift_particles.one_color = 1;
    drift_particles.one_alpha = 0;
    drift_particles.one_size = 0;
    drift_particles.alive_time = (Span) { 1, 1 };
    drift_particles.position_x = (Span) { 0, 0 };
    drift_particles.position_y = (Span) { 0, 0 };
    drift_particles.velocity = (Span) { 0.5, 0.5 };
    drift_particles.acceleration = (Span) { 0, 0 };
    drift_particles.spawn_size = (Span) { 0.1, 0.1 };
    drift_particles.spawn_red = (Span) { 0.7, 0.7 };
    drift_particles.spawn_green = (Span) { 0.7, 0.7 };
    drift_particles.spawn_blue = (Span) { 0.7, 0.7 };
    drift_particles.spawn_alpha = (Span) { 0.8, 1 };
    drift_particles.die_size = (Span) { 0.1, 0.1 };
    drift_particles.die_alpha = (Span) { 0, 0 };
    return drift_particles;
}
