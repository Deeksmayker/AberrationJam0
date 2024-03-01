#pragma once

#if DEBUG
#define assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define assert(Expression)
#endif

#define UNIT_SIZE 10.0f;

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
};

struct screen_buffer{
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel = 4;
};

struct Player{  
    Entity entity;
    b32 grounded;
    f32 base_speed = 20;
    f32 acceleration = 80;
    f32 friction = 30;
    f32 jump_force = 30;
    Vector2 velocity;
};

struct Game{
    Input input;
    f32 delta;
    f32 gravity = -30.0f;
    Array entities;
    Array walls;
    Player player;
};

struct collision_data{
    Entity *entity;  
};


void draw_rect(screen_buffer *Buffer, f32 xPosition, f32 yPosition, f32 width, f32 height, u32 color);
void draw_rect(screen_buffer *Buffer, Vector2 position, Vector2 size, u32 color);

void fill_background(screen_buffer *Buffer, u32 color);

// Time, Input, Bitmap buffer, Sound buffer
void GameUpdateAndRender(f32 delta, Input input, screen_buffer *Buffer);
void update(Game *game);
void update_player(Game *game);
void accelerate(Vector2 *velocity, f32 delta, f32 base_speed, f32 wish_speed, int direction, f32 acceleration);
void apply_friction(Vector2 *velocity, f32 delta, f32 friction);
void check_player_collisions(Game *game);
b32  check_entity_collisions(Game *game, Entity *entity, Vector2 wish_position);
b32  check_box_collision(Entity *rect1, Entity *rect2);

void render(Game *game, screen_buffer *Buffer);
void draw_entities(Game *game, screen_buffer *Buffer);