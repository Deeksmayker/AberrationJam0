#pragma once

#if DEBUG
#define assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define assert(Expression)
#endif

//6 at 1280x720
//4.5 at 960x540
//3 at 640x360
//3.75 at 800x450
#define UNIT_SIZE 3.0f;

global_variable int collisions_count = 8;
global_variable f32 unit_size = 3.0f;
global_variable u32 **splash_buffer;//[1080][972];

struct Vector2{
    f32 x;  
    f32 y;  
};

struct Entity{
    Vector2 position;  
    Vector2 scale;  
};

struct Gradient{
    u32 *colors;
    u32 colors_count;
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
    
    //Debug
    b32 g_key;
};

struct screen_buffer{
    void *Memory;
    int ScreenWidth; int ScreenHeight;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel = 4;
};

enum particle_shape{
    BoxShape = 0,  
    LineShape = 1
};

struct Particle{
    Entity entity;  
    particle_shape shape = (particle_shape)0;
    Vector2 velocity;
    Vector2 original_scale;
    f32 lifetime;
    f32 max_lifetime;
    
    b32 leave_splash;
    b32 colliding;
    
    u32 color = 0x34f132;
    u32 splash_color = 0;
};

struct particle_emitter{
    particle_shape shape = (particle_shape)0;
    b32 emitting;
    f32 emitting_timer;
    f32 per_second;
    u32 count_min = 10;
    u32 count_max = 50;
    f32 speed_min = 50;
    f32 speed_max = 100;  
    f32 scale_min = 0.1f;
    f32 scale_max = 0.5f;
    f32 spread = 0.2f;
    f32 lifetime_min = 0.5f;
    f32 lifetime_max = 2;
    
    f32 colliding_chance = 1.0f;
    
    b32 try_splash;
    f32 splash_chance;
    
    u32 color = 0x341132;
};

struct Player{  
    Entity entity;
    b32 grounded;
    b32 riding_pole;
    f32 base_speed = 40;
    f32 acceleration = 80;
    f32 air_acceleration = 60;
    f32 friction = 60;
    f32 jump_force = 45;
    f32 jump_speed_boost = 20;
    f32 gravity_multiplier = 1;
    Vector2 velocity;
    
    f32 melee_cooldown_timer;
    //shoot
    struct shooter{
        f32 pefrect_damage = 10.0f;
        f32 damage = 4.0f;
        f32 push_force = 60;
        b32 holding_shot;
        b32 holding_cleaning;
        f32 cooldown_timer;
        f32 cooldown = 0.4f;
        f32 max_holding_time = 1.0f;
        f32 holding_timer;
        f32 perfect_hold_time = 0.5f;
        f32 perfect_buffer = 0.3f;
        b32 perfect_charged;
        i32 just_shoot;
        particle_emitter wall_hit_emitter;
    };
    
    shooter shoot = {};
        
    particle_emitter pole_ride_emitter;
    particle_emitter cleaning_emitter;
    particle_emitter charged_emitter;
    particle_emitter shoot_emitter;
    
    f32 max_in_blood_time = 20;
    f32 in_blood_time;
    f32 in_blood_progress;
    f32 not_in_blood_time;
};

struct spawn_data{
    f32 spawn_time;   
    i32 fly_enemies_count;
};

global_variable int SPAWN_COUNT = 6;
global_variable int current_spawn_index = 0;
spawn_data spawns[6] = {
    {5, 1}, {15, 2}, {30, 3}, {50, 2}, {70, 2}, {100, 3}
};

enum TileType{
    None = 0,  
    Ground = 1,
    Pole = 2
};

struct Tilemap{
    u32 rows = 100;  
    u32 columns = 54;  
    u32 block_scale = 4;
};

struct Line{
    Vector2 start_position;
    Vector2 end_position;
    f32 start_width;
    f32 end_width;
};

struct line_entity{
    Line line;
    u32 color;
    f32 lifetime;
    f32 max_lifetime;
    f32 visual_start_width;
    f32 visual_end_width;
    b32 die_after_drawing;
};

struct fly_enemy_projectile{
    Entity entity;
    line_entity line_render;  
    f32 target_speed;
    f32 time_for_max_speed;
    f32 lifetime;
    Vector2 velocity;
    Vector2 direction;
};


struct fly_enemy{
    Entity entity;  
    f32 hp = 12.0f;
    b32 render_dead;
    
    b32 charging;
    f32 charge_duration = 2;
    f32 start_charge_duration = 2;
    f32 charge_timer;
    Vector2 charge_end_position;
    
    b32 strafing = 1;
    b32 picked_strafe_position;
    f32 strafe_duration = 2.0f;
    f32 start_strafe_duration = 2.0f;
    f32 strafe_distance = 40;
    f32 strafe_t;
    Vector2 strafe_start_position;
    Vector2 strafe_target_position;
    i32 max_strafe_count = 3;
    i32 strafe_count;
    
    b32 circling;
    f32 circle_speed = 5;
    f32 start_circle_speed = 5;
    f32 circle_radius = 15;
    f32 circle_angle;
    Vector2 circle_origin;
    f32 circling_time;
    
    f32 time_in_blood;
    f32 max_time_in_blood = 10;
    f32 in_blood_progress;
    
    Array lines;
    
    particle_emitter hit_emitter;
};

struct line_hits{
    int enter_count;  
    int exit_count;
    
    Vector2 *enter_positions;
    Vector2 *exit_positions;
    //1 - tile, 2 - enemy
    i32 *hit_types;
    fly_enemy **enemy_hits;
};

struct collision{
    Vector2 normal;  
    Vector2 *obstacle_velocity;
    Vector2 obstacle_position;
    
    b32 collided;
    
    union{
        TileType tile_type;  
    };
};

struct Game{
    Vector2 top_left_world_position;
    Vector2 bottom_right_world_position;
    Vector2 world_size;
    Vector2 world_pixel_size;
    Vector2 camera_pixel_position;
    Vector2 camera_player_position;
    Vector2 camera_screen_world_size;
    Input input;
    f64 time;
    f32 delta;
    f32 gravity = -60.0f;
    Array entities;
    Array walls;
    Array particles;
    Array line_entities;
    Array fly_enemies;
    Array fly_enemy_projectiles;
    Player player;
    Tilemap tilemap;
    
    f32 color_change_countdown;
    u32 current_color_palette = 0xFFFFFF;
    
    Vector2 shake_additional_position {0, 0};
    Vector2 shake_last_additional_position {0, 0};
    Vector2 shake_max_power = {6, 6.0f};
    f32 shake_trauma;
    f32 shake_trauma_decrease = 0.7f;
    
    b32 im_dying_man = 0;
    
    particle_emitter blood_emitter;
    particle_emitter dust_emitter;
    
    Gradient background_gradient;
    Gradient darker_background_gradient;
    Gradient tiles_gradient;
    Gradient blood_gradient;
    Gradient pole_gradient;
    Gradient fly_enemies_gradient;
};

global_variable Vector2 camera_position;

f32 hitstop_countdown = 0;

void draw_world_rect(screen_buffer *Buffer, Vector2 position, Vector2 scale, Gradient gradient);
void draw_rect(screen_buffer *Buffer, f32 xPosition, f32 yPosition, f32 width, f32 height, u32 color);
void draw_rect(screen_buffer *Buffer, Vector2 position, Vector2 size, u32 color);
void draw_line(screen_buffer *Buffer, f32 start_x, f32 start_y, f32 end_x, f32 end_y, f32 width, u32 color);
void draw_line(screen_buffer *Buffer, Vector2 start_position, Vector2 end_position, f32 width, u32 color);
void draw_line(screen_buffer *Buffer, Line line, u32 color);
void draw_line(screen_buffer *Buffer, line_entity line_e); 

f32 get_random_fraction_from_pixel_position(Game *game, i32 x, i32 y, f32 fraction);
void fill_background(screen_buffer *Buffer, Gradient gradient);

// Time, Input, Bitmap buffer, Sound buffer
void GameUpdateAndRender(f32 delta, Input input, screen_buffer *Buffer);
void update(Game *game);
void update_player(Game *game);
void update_enemies_spawn(Game *game);
void update_fly_enemies(Game *game);
void update_fly_enemy_projectiles(Game *game);
void spawn_fly_enemy_projectile(Game *game, Vector2 direction, Vector2 start_position);
void update_particles(Game *game);
void accelerate(Vector2 *velocity, f32 delta, f32 base_speed, f32 wish_speed, int direction, f32 acceleration);
void apply_friction(Vector2 *velocity, f32 max_speed, f32 delta, f32 friction);
void check_player_collisions(Game *game);
fly_enemy *check_enemy_collision(Game *game, Entity entity);
void calculate_player_tilemap_collisions(Game *game, collision *collisions_data);
void calculate_particle_tilemap_collisions(Game *game, Particle *particle, collision *collisions_data);
void shoot_rifle(Game *game, Player *player, Player::shooter *shoot, Vector2 player_to_mouse, b32 perfect_shoot);
collision *check_tilemap_collisions(Game *game, Vector2 velocity, Entity entity);
collision check_particles_collisions(Game *game, Vector2 velocity, Entity entity);
b32 check_entity_collisions(Game *game, Entity *entity, Vector2 wish_position);
b32 check_box_collision(Entity *rect1, Entity *rect2);
line_hits check_line_collision(Game *game, Line line);

void shake_camera(Game *game, f32 stress);
void update_camera_shake(Game *game);


void add_fly_enemy(Game *game, Vector2 position);

void debug_update(Game *game);

void render(Game *game, screen_buffer *Buffer);
void draw_entities(Game *game, screen_buffer *Buffer);

void update_overtime_emitter(Game *game, particle_emitter *emitter, Vector2 direction, Vector2 start_position, f32 count_multiplier);
void emit_particles(Game *game, particle_emitter emmiter, Vector2 direction, Vector2 start_position, f32 count_multiplier);
void shoot_particle(Game *game, particle_emitter emitter, Vector2 direction, Vector2 start_position);


int level1[100][54] = {
    {0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,2,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,2,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,2,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,2,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,2,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,2,0,0,0,0,0,0,2,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,1,0,2,0,0,0,0,0,0,2,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,1,0,2,0,0,0,0,0,0,2,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,2,0,0,1,0,2,0,0,0,0,0,0,2,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,1,0,2,0,0,0,0,0,0,2,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,1,0,2,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,1,0,2,0,0,1,0,0,0,0,0,0},
    {1,1,1,1,1,1,1,1,1,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,2,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,2,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,2,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,2,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,2,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,2,1,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {0,0,2,1,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2},
    {0,0,2,1,0,0,0,0,0,1,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,2},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,0,2,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,2,2},
    {0,0,0,0,0,0,0,0,1,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,2,2,2},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,2,2,2,2},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,2,2,2,2,2},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,2,2,2,2,2,2},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,2,2,2,2,2,2,2},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,2,2,2,2,2,2,2,2},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,2,2,2,2,2,2,2,2,2,2},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,2,2,2,2,2,2,2,2,2,2,2},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,2,2,2,2,2,2,2,2,2,2,2,2},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,2,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {0,0,0,0,0,0,0,0,0,0,0,1,1,0,2,0,1,1,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,1,1,0,0,2,0,0,1,1,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,1,1,0,0,0,2,0,0,0,1,1,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,1,0,0,0,0,2,0,0,0,0,1,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,1,0,0,0,0,0,2,0,0,0,0,0,1,1,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,1,0,0,0,0,0,2,0,0,0,0,0,1,1,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,1,0,0,0,0,2,0,0,0,0,1,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,1,1,0,0,0,2,0,0,0,1,1,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,1,1,0,0,2,0,0,1,1,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,1,0,2,0,1,1,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,2,2,1,1,2,2,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,2,0,1,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,2,1,1,1,1,2,2,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,2,2,1,1,1,1,1,1,2,2,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,2,2,1,1,1,1,1,1,1,1,2,2,0,0,0,0,0,0,0},
    {0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,2,2,1,1,1,1,1,1,1,1,1,1,2,2,2,0,0,0,0,0},
    {1,1,0,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,2,1,1,1,1,1,1,1,1,1,1,1,1,2,2,0,0,0,1,1},
    {0,0,0,0,2,1,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0},
    {0,0,0,0,2,1,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,1,0,0,0,2,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0},
    {0,0,0,0,2,1,1,1,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,1,1,0,0,0,2,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,0,0,2,0,0,0,0,0,0,0,1,1,0,0,0,0,2,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,2,1,1,0,0,0,0,0,2,0,0,0,0,0,1,1,2,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,2,1,0,0,0,0,0,0,2,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,1,0,2,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,2,1,0,0,0,0,0,0,2,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,1,0,2,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,2,1,0,0,0,0,0,0,2,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,2,1,0,0,0,0,0,0,2,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,1,0,0,2,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,2,1,0,0,0,0,0,0,2,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,1,0,0,2,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,2,1,0,0,0,0,0,0,2,0,0,0,0,0,0,1,2,2,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,2,1,0,0,0,0,0,0,2,0,0,0,0,0,0,1,2,2,0,0,0,0,0,0,1,0,2,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,2,1,0,0,0,0,0,0,2,0,0,0,0,0,0,1,2,2,0,0,0,0,0,0,1,0,2,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,2,0,0,0,0,0,0,0,2,0,0,0,0,0,2,1,0,0,0,0,0,0,2,0,0,0,0,0,0,1,2,2,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0},
    {1,1,1,1,1,0,2,0,1,1,0,0,0,0,2,0,0,0,0,0,2,1,0,0,0,0,0,0,2,0,0,0,0,0,0,1,2,2,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1},
    {0,0,0,0,0,0,2,0,0,0,0,0,0,0,2,0,0,0,0,0,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,2,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,2,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,2,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,1,1,1,0,2,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
    {0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0},
    {0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0},
    {0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0},
    {0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0},
    {0,0,2,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0},
    {0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0},
    {0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0},
    {0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0},
    {0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0},
    {0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0},
    {0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0},
    {0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0},
    {0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0},
    {0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0},
    {0,0,2,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0},
    {0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0},
    {0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0}
};








