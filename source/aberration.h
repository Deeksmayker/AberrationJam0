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
    
    Vector2 mouse_screen_position;
    Vector2 mouse_world_position;
    b32 mouse_left_key;
};

struct screen_buffer{
    void *Memory;
    int ScreenWidth; int ScreenHeight;
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
    f32 air_acceleration = 60;
    f32 friction = 60;
    f32 jump_force = 30;
    f32 jump_speed_boost = 20;
    Vector2 velocity;
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

struct Game{
    f32 unit_size = 10.0f;
    Input input;
    f32 delta;
    f32 gravity = -60.0f;
    Array entities;
    Array walls;
    Player player;
    Tilemap tilemap;
};

struct collision_data{
    Entity *entity;  
};

global_variable Vector2 camera_position;

void draw_rect(screen_buffer *Buffer, f32 xPosition, f32 yPosition, f32 width, f32 height, u32 color);
void draw_rect(screen_buffer *Buffer, Vector2 position, Vector2 size, u32 color);

void fill_background(screen_buffer *Buffer, u32 color);

// Time, Input, Bitmap buffer, Sound buffer
void GameUpdateAndRender(f32 delta, Input input, screen_buffer *Buffer);
void update(Game *game);
void update_player(Game *game);
void accelerate(Vector2 *velocity, f32 delta, f32 base_speed, f32 wish_speed, int direction, f32 acceleration);
void apply_friction(Vector2 *velocity, f32 max_speed, f32 delta, f32 friction);
void check_player_collisions(Game *game);
b32  check_entity_collisions(Game *game, Entity *entity, Vector2 wish_position);
b32  check_box_collision(Entity *rect1, Entity *rect2);

void fill_level1_tilemap(Game *game);

void render(Game *game, screen_buffer *Buffer);
void draw_entities(Game *game, screen_buffer *Buffer);



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
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,1,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};








