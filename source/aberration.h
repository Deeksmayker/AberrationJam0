#pragma once

#if DEBUG
#define assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define assert(Expression)
#endif

#define UNIT_SIZE 10.0f;

global_variable int collisions_count = 8;
global_variable f32 unit_size = 7.0f;

struct Vector2{
    f32 x;  
    f32 y;  
};

struct Entity{
    Vector2 position;  
    Vector2 scale;  
};

struct Input{
    b32 up_key;  
    b32 down_key;
    b32 right_key;
    b32 left_key;
    b32 jump_key;
    
    Vector2 mouse_screen_position;
    Vector2 mouse_world_position;
    b32 mouse_left_key;
    b32 mouse_right_key;
};

struct screen_buffer{
    void *Memory;
    int ScreenWidth; int ScreenHeight;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel = 4;
};

struct Particle{
    Entity entity;  
    Vector2 velocity;
    Vector2 original_scale;
    f32 lifetime;
    f32 max_lifetime;
};

struct particle_emitter{
    b32 emitting;
    u32 per_second;
    u32 count_min = 10;
    u32 count_max = 50;
    f32 speed_min = 50;
    f32 speed_max = 100;  
    f32 scale_min = 0.5f;
    f32 scale_max = 2;
    f32 spread = 0.2f;
    f32 lifetime_min = 0.5f;
    f32 lifetime_max = 2;
    
    u32 color = 0x34f132;
};

struct Player{  
    Entity entity;
    b32 grounded;
    b32 riding_pole;
    f32 base_speed = 20;
    f32 acceleration = 80;
    f32 air_acceleration = 60;
    f32 friction = 60;
    f32 jump_force = 45;
    f32 jump_speed_boost = 20;
    f32 gravity_multiplier = 1;
    Vector2 velocity;
    
    f32 melee_cooldown_timer;
    particle_emitter shoot_emitter;
};

enum TileType{
    None = 0,  
    Ground = 1,
    Pole = 2
};

struct Tilemap{
    u32 rows = 30;  
    u32 columns = 30;  
    u32 block_scale = 4;
};

struct collision{
    Vector2 normal;  
    Vector2 *obstacle_velocity;
    
    b32 collided;
    
    union{
        TileType tile_type;  
    };
};

struct Game{
    Input input;
    f64 time;
    f32 delta;
    f32 gravity = -60.0f;
    Array entities;
    Array walls;
    Array particles;
    Player player;
    Tilemap tilemap;
};

global_variable Vector2 camera_position;

void draw_rect(screen_buffer *Buffer, f32 xPosition, f32 yPosition, f32 width, f32 height, u32 color);
void draw_rect(screen_buffer *Buffer, Vector2 position, Vector2 size, u32 color);

void fill_background(screen_buffer *Buffer, u32 color);

// Time, Input, Bitmap buffer, Sound buffer
void GameUpdateAndRender(f32 delta, Input input, screen_buffer *Buffer);
void update(Game *game);
void update_player(Game *game);
void update_particles(Game *game);
void accelerate(Vector2 *velocity, f32 delta, f32 base_speed, f32 wish_speed, int direction, f32 acceleration);
void apply_friction(Vector2 *velocity, f32 max_speed, f32 delta, f32 friction);
void check_player_collisions(Game *game);
void calculate_player_tilemap_collisions(Game *game, collision *collisions_data);
void calculate_particle_tilemap_collisions(Game *game, Particle *particle, collision *collisions_data);
collision *check_tilemap_collisions(Game *game, Vector2 velocity, Entity entity);
collision check_particles_collisions(Game *game, Vector2 velocity, Entity entity);
b32  check_entity_collisions(Game *game, Entity *entity, Vector2 wish_position);
b32  check_box_collision(Entity *rect1, Entity *rect2);

void debug_update(Game *game);

void fill_level1_tilemap(Game *game);

void render(Game *game, screen_buffer *Buffer);
void draw_entities(Game *game, screen_buffer *Buffer);

void emit_particles(Game *game, particle_emitter emmiter, Vector2 direction, Vector2 start_position);
void shoot_particle(Game *game, particle_emitter emitter, Vector2 direction, Vector2 start_position, u32 seed_multiplier);


int level1[30][30] = {
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,1,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};








