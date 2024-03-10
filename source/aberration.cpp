global_variable u32 rand_seed = 0;

#include "utils.cpp"
#include "aberration.h"
#include "my_math.cpp"

Game global_game;

Vector2 get_camera_position(){
    return add(camera_position, global_game.shake_additional_position);
}

internal void
RenderFunnyGradient(screen_buffer *Buffer, int XOffset, int YOffset){
    u8 *ROW = (u8 *) Buffer->Memory;
    for (int Y = 0; Y < Buffer->Height; Y++){    
        u32 *Pixel = (u32 *) ROW;
        for (int X = 0; X < Buffer->Width; X++){
            u8 blue = (X + XOffset);
            u8 green = (Y + YOffset);
            *Pixel++ = (u32)((green << 8) | blue);
        }
        ROW += Buffer->Pitch;
    }
}


void loop_world_position(Game *game, Vector2 *position){
    while (position->y < game->bottom_right_world_position.y){
        position->y += game->top_left_world_position.y;
    }
    
    while (position->y >= game->top_left_world_position.y){
        position->y -= game->top_left_world_position.y;
    }
    
    while (position->x < game->top_left_world_position.x){
        position->x += game->bottom_right_world_position.x;
    }
    
    while (position->x >= game->bottom_right_world_position.x){
        position->x -= game->bottom_right_world_position.x;
    }
}

void draw_world_rect(Game *game, screen_buffer *Buffer, Vector2 position, Vector2 scale, Gradient gradient){
    f32 screen_world_height = (f32)Buffer->Height / unit_size;

    //position.x -= camera_position.x;
    //position.y -= camera_position.y;
    
    position.x -= get_camera_position().x;
    position.y -= get_camera_position().y;

    
    Vector2 world_position = {position.x, position.y};
    loop_world_position(&global_game, &world_position);
    position.x = world_position.x;
    position.y = world_position.y;

    scale.x  *= unit_size;
    scale.y *= unit_size;
    position.x *= unit_size;
    position.y *= unit_size;
    u8 *row = (u8 *) Buffer->Memory;
    
    i32 upPosition     = clamp((i32)position.y + (i32)(scale.y * 0.5f), 0, Buffer->Height - 1);
    i32 bottomPosition = clamp((i32)position.y - (i32)(scale.y * 0.5f), 0, Buffer->Height - 1);
    i32 leftPosition   = clamp((i32)position.x - (i32)(scale.x * 0.5f), 0, Buffer->Width - 1);
    i32 rightPosition  = clamp((i32)position.x + (i32)(scale.x * 0.5f), 0, Buffer->Width - 1);
    
    row += Buffer->Pitch * bottomPosition;
    
    for (int y = bottomPosition; y <= upPosition; y++){
        if (y >= Buffer->Height || y < 0){
            continue;
        }
        u32 *pixel = (u32 *)row;
        pixel += leftPosition;
        
        i32 y_pixel_position = y + game->camera_pixel_position.y;
        f32 fraction = (y_pixel_position) / game->world_pixel_size.y;
        
        for (int x = leftPosition; x <= rightPosition; x++){
            if (x >= Buffer->Width || x < 0){
                continue;
            }
            
            f32 real_fraction = get_random_fraction_from_pixel_position(game, x, y_pixel_position, fraction);
            u32 color = lerp_gradient(gradient, real_fraction);                  
            
            color &= game->current_color_palette;
            color = lerp_color(color, 0xFF0A0A, sqrt(game->player.in_blood_progress));
            
            *pixel++ = color;
        }
        row += Buffer->Pitch;
    }
    
}


void draw_rect(screen_buffer *Buffer, f32 xPosition, f32 yPosition, f32 width, f32 height, u32 color){
    f32 screen_world_height = (f32)Buffer->Height / unit_size;

    //xPosition -= camera_position.x;
    //yPosition -= camera_position.y;
    
    xPosition -= get_camera_position().x;
    yPosition -= get_camera_position().y;

    
    if (yPosition < 0 && abs(yPosition + screen_world_height) < screen_world_height){
//        yPosition = abs(yPosition) - screen_world_height;
    }
    
    Vector2 world_position = {xPosition, yPosition};
    loop_world_position(&global_game, &world_position);
    xPosition = world_position.x;
    yPosition = world_position.y;

    width  *= unit_size;
    height *= unit_size;
    xPosition *= unit_size;
    yPosition *= unit_size;
    u8 *row = (u8 *) Buffer->Memory;
    
    i32 upPosition     = clamp((i32)yPosition + (i32)(height * 0.5f), 0, Buffer->Height - 1);
    i32 bottomPosition = clamp((i32)yPosition - (i32)(height * 0.5f), 0, Buffer->Height - 1);
    i32 leftPosition   = clamp((i32)xPosition - (i32)(width * 0.5f), 0, Buffer->Width - 1);
    i32 rightPosition  = clamp((i32)xPosition + (i32)(width * 0.5f), 0, Buffer->Width - 1);
    
    row += Buffer->Pitch * bottomPosition;
    
    for (int y = bottomPosition; y <= upPosition; y++){
        if (y >= Buffer->Height || y < 0){
            continue;
        }
        u32 *pixel = (u32 *)row;
        pixel += leftPosition;
        for (int x = leftPosition; x <= rightPosition; x++){
            if (x >= Buffer->Width || x < 0){
                continue;
            }
                  
            *pixel++ = color;
        }
        row += Buffer->Pitch;
    }
}

void draw_rect(screen_buffer *Buffer, Vector2 position, Vector2 size, u32 color){
    draw_rect(Buffer, position.x, position.y, size.x, size.y, color);
}

void draw_line(screen_buffer *Buffer, f32 start_x, f32 start_y, f32 end_x, f32 end_y, f32 start_width, f32 end_width, u32 color){
    Vector2 vector_to_end = subtract({end_x, end_y}, {start_x, start_y});
    int points_count = (((int)magnitude(vector_to_end) + 10) * 4);
    
    for (int i = 0; i <= points_count; i++){
        f32 t = lerp((f32)0, (f32)1, (f32)i / (f32)points_count);
        
        f32 x = lerp(start_x, end_x, t);
        f32 y = lerp(start_y, end_y, t);
        
        f32 block_width = lerp(start_width, end_width, t);
        
        draw_rect(Buffer, x, y, block_width, block_width, color);
    }
}

void draw_line_gradient(Game *game, screen_buffer *Buffer, f32 start_x, f32 start_y, f32 end_x, f32 end_y, f32 start_width, f32 end_width, Gradient gradient){
    Vector2 vector_to_end = subtract({end_x, end_y}, {start_x, start_y});
    int points_count = (((int)magnitude(vector_to_end) + 10) * 4);
    
    for (int i = 0; i <= points_count; i++){
        f32 t = lerp((f32)0, (f32)1, (f32)i / (f32)points_count);
        
        f32 x = lerp(start_x, end_x, t);
        f32 y = lerp(start_y, end_y, t);
        
        f32 block_width = lerp(start_width, end_width, t);
        
        draw_world_rect(game, Buffer, {x, y}, {block_width, block_width}, gradient);
    }
    
}

void draw_line_gradient(Game *game, screen_buffer *Buffer, Vector2 start_position, Vector2 end_position, f32 start_width, f32 end_width, Gradient gradient){
    draw_line_gradient(game, Buffer, start_position.x, start_position.y, end_position.x, end_position.y, start_width, end_width, gradient);    
}

void draw_line(screen_buffer *Buffer, Vector2 start_position, Vector2 end_position, f32 start_width, f32 end_width, u32 color){
    draw_line(Buffer, start_position.x, start_position.y, end_position.x, end_position.y, start_width, end_width, color);
}

void draw_line(screen_buffer *Buffer, Line line, u32 color){
    draw_line(Buffer, line.start_position.x, line.start_position.y, line.end_position.x, line.end_position.y, line.start_width, line.end_width, color);
}

void draw_line(screen_buffer *Buffer, line_entity line_e){
    draw_line(Buffer, line_e.line.start_position, line_e.line.end_position, line_e.visual_start_width, line_e.visual_end_width, line_e.color);
    
}

f32 get_random_fraction_from_pixel_position(Game *game, i32 x, i32 y, f32 fraction){
    f32 reduction_power = game->im_dying_man ? 2048.0 : 4096.0;
    
    i32 random_value = 0;
    
    if (!game->im_dying_man){
        random_value = (rnd((x & 0xFFFF) << 16 | ((i32)y & 0xFFFF)) + (i32)(game->time * 10)) % 128;
        random_value -= 64;
    } else{
        random_value = rnd() % 128;
    }
    
    f32 result = fraction + (f32)random_value  / reduction_power;
    
    if (result < 0){
        result *= -1;
    }
    
    if (result > 1){
        result -= 1;
    }
    
    return result;
}

void fill_background(Game *game, screen_buffer *Buffer, Gradient gradient){
    u8 *ROW = (u8 *) Buffer->Memory;
    for (int y = 0; y < Buffer->Height; y++){
        u32 *Pixel = (u32 *) ROW;
        Vector2 pixel_world_position = {game->camera_pixel_position.x / unit_size, ((f32)y + game->camera_pixel_position.y) / unit_size};
        loop_world_position(game, &pixel_world_position);
        
        f32 y_pixel_position = pixel_world_position.y * unit_size;
        
        f32 fraction = (y_pixel_position) / game->world_pixel_size.y;

        for (int x = 0; x < Buffer->Width; x++){
            
            f32 real_fraction = get_random_fraction_from_pixel_position(game, x, y_pixel_position, fraction);
            u32 color = lerp_gradient(gradient, real_fraction);                  
            
            color &= game->current_color_palette;
            color = lerp_color(color, 0xFF0A0A, sqrt(game->player.in_blood_progress));
            
            *Pixel++ = color;
        }
        ROW += Buffer->Pitch;
    }
}

void draw_splashes(Game *game, screen_buffer *Buffer){
    i32 up_splashes_index   = get_camera_position().y * unit_size + Buffer->Height * 0.5f;
    i32 down_splashes_index = get_camera_position().y * unit_size - Buffer->Height * 0.5f;
    
    i32 additional_x = get_camera_position().x * unit_size;
    
    u8 *ROW = (u8 *) Buffer->Memory;
    for (int y = 0; y < Buffer->Height; y++){
        i32 y_world_index = y / unit_size + get_camera_position().y - 1;
        
        Vector2 world_y_index_position = {0, (f32)y_world_index};
        loop_world_position(game, &world_y_index_position);
        i32 y_index = world_y_index_position.y * unit_size;// + y;
        f32 fraction = (y_index) / game->world_pixel_size.y;
    
        u32 *Pixel = (u32 *) ROW;
        for (int x = 0; x < Buffer->Width; x++){
            
            if (y_index >= game->world_size.y * unit_size || splash_buffer[y_index][x + additional_x] == 0){
                Pixel++;
                continue;
            }
            
            f32 real_fraction = get_random_fraction_from_pixel_position(game, x + additional_x, y_index, fraction);
            u32 color = lerp_gradient(game->blood_gradient, real_fraction);                  
            
            *Pixel++ = color;//splash_buffer[y_index][x];                  
        }
        ROW += Buffer->Pitch;
    }
}

void add_splash(Game *game, Entity entity, u32 color){
    i32 up_pixel_position     = (i32)((entity.position.y + entity.scale.y * 0.5f) * unit_size);
    i32 bottom_pixel_position = (i32)((entity.position.y - entity.scale.y * 0.5f) * unit_size);
    i32 left_pixel_position   = (i32)((entity.position.x - entity.scale.x * 0.5f) * unit_size);
    i32 right_pixel_position  = (i32)((entity.position.x + entity.scale.x * 0.5f) * unit_size);
    
    if (color == 0xffffff) color = 0;
    
    for (int y = bottom_pixel_position; y <= up_pixel_position; y++){
        for (int x = left_pixel_position; x <= right_pixel_position; x++){
            Vector2 world_pixel_position = {((f32)x) / unit_size , ((f32)y) / unit_size};
            loop_world_position(game, &world_pixel_position);
            i32 x_pixel = world_pixel_position.x * unit_size;
            i32 y_pixel = world_pixel_position.y * unit_size;
            
            //xdd
            //Positions - bottom to top, but pixels is top to bottom, so we subtract height from current index
            //We need this to properly convert world positions to splash buffer indexes
            //y_pixel = global_game.world_size.y * unit_size - y_pixel - 1;
            
            splash_buffer[y_pixel][x_pixel] = color;
        }
    }
}

b32 in_blood(Entity entity){
    if (splash_buffer[(i32)(entity.position.y * unit_size)][(i32)(entity.position.x * unit_size)] != 0){
        return 1;
    }
    
    return 0;
}


void InitGame(){
    global_game = {};
    global_game.entities = array_init(sizeof(Entity));
    
    
    global_game.player = {};
    global_game.player.entity = {};
    global_game.player.entity.position = {30, 30};
    global_game.player.entity.scale = {3, 3};
    
    global_game.particles = array_init(sizeof(Particle), 100000);
    global_game.line_entities = array_init(sizeof(line_entity), 100000);
    
    global_game.fly_enemies = array_init(sizeof(fly_enemy), 100000);
    global_game.fly_enemy_projectiles = array_init(sizeof(fly_enemy_projectile), 100000);
    
    //add_fly_enemy(&global_game, {30, 35});
    
    /*
    global_game.walls = array_init(sizeof(Entity));
    
    Entity floor = {};
    floor.position = {60, 10};
    floor.scale = {100, 10};
    array_add(&global_game.walls, &floor);
    
    Entity right_wall = {};
    right_wall.position = {100, 30};
    right_wall.scale = {5, 150};
    array_add(&global_game.walls, &right_wall);
    */
    
    global_game.tilemap = {};
    
    global_game.top_left_world_position     = {0, (f32)global_game.tilemap.rows * global_game.tilemap.block_scale};
    global_game.bottom_right_world_position = {(f32)global_game.tilemap.columns * global_game.tilemap.block_scale, 0};
    global_game.world_size = {global_game.bottom_right_world_position.x, global_game.top_left_world_position.y};
    global_game.world_pixel_size = multiply(global_game.world_size, unit_size);
    
    splash_buffer = (u32 **)malloc(global_game.world_size.y * unit_size * sizeof(u32 *));
    for (int i = 0; i < global_game.world_size.y * unit_size; i++){
        splash_buffer[i] = (u32 *)calloc(global_game.world_size.x * unit_size, sizeof(u32));
    }
    
    u32 sunset_orange = 0xfd754d;
    u32 sky_white = 0xefeeee;
    u32 blue_sky = 0x98bad5;
    
    u32 pale_green = 0x98fb98;
    u32 bamboo_green = 0xe2dbac;
    u32 dark_green = 0x2d4b2d;
    
    particle_emitter *wall_hit_emitter = &global_game.player.shoot.wall_hit_emitter;
    *wall_hit_emitter = {};
    wall_hit_emitter->speed_min        = 40;
    wall_hit_emitter->speed_max        = 100;
    wall_hit_emitter->scale_min        = 0.9f;
    wall_hit_emitter->scale_max        = 2.0f;
    wall_hit_emitter->count_min        = 10;
    wall_hit_emitter->count_max        = 30;
    wall_hit_emitter->lifetime_min     = 0.5f;
    wall_hit_emitter->lifetime_max     = 1.5f;
    wall_hit_emitter->shape = (particle_shape)0;
    wall_hit_emitter->color            = dark_green;
    wall_hit_emitter->spread           = 0.4f;
    wall_hit_emitter->shape            = (particle_shape)0;
    wall_hit_emitter->try_splash       = 0;
    wall_hit_emitter->splash_chance    = 0;
    wall_hit_emitter->colliding_chance = 1;
    wall_hit_emitter->per_second       = 1;
    
    particle_emitter *pole_ride_emitter = &global_game.player.pole_ride_emitter;
    *pole_ride_emitter = {};
    pole_ride_emitter->speed_min    = 20;
    pole_ride_emitter->speed_max    = 60;
    pole_ride_emitter->scale_min    = 0.1f;
    pole_ride_emitter->scale_max    = 0.2f;
    pole_ride_emitter->lifetime_min = 0.3f;
    pole_ride_emitter->lifetime_max = 0.7f;
    pole_ride_emitter->per_second = 80;
    pole_ride_emitter->color = 0xffff22;
    pole_ride_emitter->spread = 1;
    pole_ride_emitter->shape = (particle_shape)1;
    
    particle_emitter *blood_emitter = &global_game.blood_emitter;
    *blood_emitter = {};
    blood_emitter->speed_min    = 20;
    blood_emitter->speed_max    = 60;
    blood_emitter->scale_min    = 0.4f;
    blood_emitter->scale_max    = 1.0f;
    blood_emitter->lifetime_min = 0.6f;
    blood_emitter->lifetime_max = 1.5f;
    blood_emitter->color = 0xaa1133;
    blood_emitter->spread = 1;
    blood_emitter->shape = (particle_shape)1;
    blood_emitter->try_splash = 1;
    blood_emitter->splash_chance = 0.5f;
    blood_emitter->colliding_chance = 0.8f;
    blood_emitter->per_second = 10;
    
    particle_emitter *cleaning_emitter = &global_game.player.cleaning_emitter;
    *cleaning_emitter = {};
    cleaning_emitter->count_min        = 30;
    cleaning_emitter->count_max        = 80;
    cleaning_emitter->speed_min        = 40;
    cleaning_emitter->speed_max        = 150;
    cleaning_emitter->scale_min        = 1.0f;
    cleaning_emitter->scale_max        = 3.0f;
    cleaning_emitter->lifetime_min     = 1.0f;
    cleaning_emitter->lifetime_max     = 2.0f;
    cleaning_emitter->color            = 0xffffff;
    cleaning_emitter->spread           = 0.4f;
    cleaning_emitter->shape            = (particle_shape)1;
    cleaning_emitter->try_splash       = 1;
    cleaning_emitter->splash_chance    = 0.8f;
    cleaning_emitter->colliding_chance = 1.0f;
    cleaning_emitter->per_second       = 10;
    
    u32 sun_yellow = 0xFFDF22;
    
    particle_emitter *charged_emitter = &global_game.player.charged_emitter;
    *charged_emitter = {};
    charged_emitter->count_min        = 10;
    charged_emitter->count_max        = 30;
    charged_emitter->speed_min        = 30;
    charged_emitter->speed_max        = 70;
    charged_emitter->scale_min        = 0.5;
    charged_emitter->scale_max        = 1.2f;
    charged_emitter->lifetime_min     = 0.5f;
    charged_emitter->lifetime_max     = 1.0f;
    charged_emitter->color            = sun_yellow;
    charged_emitter->spread           = 0.4f;
    charged_emitter->shape            = (particle_shape)1;
    charged_emitter->try_splash       = 0;
    charged_emitter->splash_chance    = 0;
    charged_emitter->colliding_chance = 0;
    charged_emitter->per_second       = 10;
    
    particle_emitter *shoot_emitter = &global_game.player.shoot_emitter;
   *shoot_emitter = {};
    shoot_emitter->count_min        = 1;
    shoot_emitter->count_max        = 1;
    shoot_emitter->speed_min        = 30;
    shoot_emitter->speed_max        = 100;
    shoot_emitter->scale_min        = 1.0f;
    shoot_emitter->scale_max        = 1.5f;
    shoot_emitter->lifetime_min     = 10.0f;
    shoot_emitter->lifetime_max     = 20.0f;
    shoot_emitter->color            = sunset_orange;
    shoot_emitter->spread           = 0.4f;
    shoot_emitter->shape            = (particle_shape)0;
    shoot_emitter->try_splash       = 0;
    shoot_emitter->splash_chance    = 0;
    shoot_emitter->colliding_chance = 1;
    shoot_emitter->per_second       = 1;
    
    particle_emitter *dust_emitter = &global_game.dust_emitter;
   *dust_emitter = {};
    dust_emitter->count_min        = 10;
    dust_emitter->count_max        = 30;
    dust_emitter->speed_min        = 30;
    dust_emitter->speed_max        = 50;
    dust_emitter->scale_min        = 0.5f;
    dust_emitter->scale_max        = 1.0f;
    dust_emitter->lifetime_min     = 0.5f;
    dust_emitter->lifetime_max     = 1.0f;
    dust_emitter->color            = 0xa0a0a0;
    dust_emitter->spread           = 0.8f;
    dust_emitter->shape            = (particle_shape)0;
    dust_emitter->try_splash       = 0;
    dust_emitter->splash_chance    = 0;
    dust_emitter->colliding_chance = 0.5f;
    dust_emitter->per_second       = 10;
    
    global_game.background_gradient = {};
    global_game.background_gradient.colors = (u32 *)malloc(5 * sizeof(u32));
    global_game.background_gradient.colors[0] = blue_sky;
    global_game.background_gradient.colors[1] = sky_white;
    global_game.background_gradient.colors[2] = sunset_orange;
    global_game.background_gradient.colors[3] = sky_white;
    global_game.background_gradient.colors[4] = blue_sky;
    global_game.background_gradient.colors_count = 5;
    
    global_game.darker_background_gradient = {};
    global_game.darker_background_gradient.colors = (u32 *)malloc(5 * sizeof(u32));
    global_game.darker_background_gradient.colors[0] = blue_sky - 0x0a0a0a;
    global_game.darker_background_gradient.colors[1] = sky_white - 0x0a0a0a;
    global_game.darker_background_gradient.colors[2] = sunset_orange - 0x0a0a0a;
    global_game.darker_background_gradient.colors[3] = sky_white - 0x0a0a0a;
    global_game.darker_background_gradient.colors[4] = blue_sky - 0x0a0a0a;
    global_game.darker_background_gradient.colors_count = 5;
    
    u32 grass = 0x41980a;
    u32 pale_grass = 0xe3fd98;
    u32 ground = 0x87421c;
    
    global_game.tiles_gradient = {};
    global_game.tiles_gradient.colors = (u32 *)malloc(5 * sizeof(u32));
    global_game.tiles_gradient.colors[0] = grass;
    global_game.tiles_gradient.colors[1] = pale_grass;
    global_game.tiles_gradient.colors[2] = ground;
    global_game.tiles_gradient.colors[3] = pale_grass;
    global_game.tiles_gradient.colors[4] = grass;
    global_game.tiles_gradient.colors_count = 5;
    
    u32 blood_light = 0xdb0000;
    u32 blood_medium = 0xbb6350;
    u32 blood_dark = 0x750000;
    
    global_game.blood_gradient = {};
    global_game.blood_gradient.colors = (u32 *)malloc(5 * sizeof(u32));
    global_game.blood_gradient.colors[0] = blood_light;
    global_game.blood_gradient.colors[1] = blood_medium;
    global_game.blood_gradient.colors[2] = blood_dark;
    global_game.blood_gradient.colors[3] = blood_medium;
    global_game.blood_gradient.colors[4] = blood_light;
    global_game.blood_gradient.colors_count = 5;
    
    global_game.pole_gradient = {};
    global_game.pole_gradient.colors = (u32 *)malloc(5 * sizeof(u32));
    global_game.pole_gradient.colors[0] = pale_green;
    global_game.pole_gradient.colors[1] = bamboo_green;
    global_game.pole_gradient.colors[2] = dark_green;
    global_game.pole_gradient.colors[3] = bamboo_green;
    global_game.pole_gradient.colors[4] = pale_green;
    global_game.pole_gradient.colors_count = 5;
    
    u32 pale_pink = 0xfadadd;
    u32 violet = 0xbf8bff;
    
    global_game.fly_enemies_gradient = {};
    global_game.fly_enemies_gradient.colors = (u32 *)malloc(5 * sizeof(u32));
    global_game.fly_enemies_gradient.colors[0] = blood_dark;
    global_game.fly_enemies_gradient.colors[1] = violet;
    global_game.fly_enemies_gradient.colors[2] = pale_pink;
    global_game.fly_enemies_gradient.colors[3] = violet;
    global_game.fly_enemies_gradient.colors[4] = blood_dark;
    global_game.fly_enemies_gradient.colors_count = 5;
}


global_variable f32 previous_delta = 0;

void GameUpdateAndRender(f32 delta, Input input, screen_buffer *Buffer){
/*
    local_persist b32 previous_g = 0;
    if (input.g_key && !previous_g){
        if (game_width_pixels == 640){
            unit_size = 3.0f;
        } else{
            unit_size = 3.75f;
        }
    }
    previous_g = input.g_key;
*/
    rand_seed = (u32)(global_game.delta * 10000000000.0f);
    rnd_state = rand_seed;
    
    global_game.camera_screen_world_size = {Buffer->Width / unit_size, Buffer->Height / unit_size};
    
    f32 mouse_world_position_x = ((f32)input.mouse_screen_position.x /
                                 ((f32)Buffer->ScreenWidth / (f32)Buffer->Width)) / 
                                 unit_size;
    f32 mouse_world_position_y = ((f32)input.mouse_screen_position.y /
                                 ((f32)Buffer->ScreenHeight / (f32)Buffer->Height)) / 
                                 unit_size;
                                 
    input.mouse_world_position = {mouse_world_position_x, mouse_world_position_y + camera_position.y};
    
    global_game.camera_pixel_position = multiply(get_camera_position(), unit_size);
    
    f32 screen_world_height = (f32)Buffer->Height / unit_size;
    if(global_game.top_left_world_position.y - get_camera_position().y < screen_world_height
                            && global_game.player.entity.position.y < screen_world_height){
        global_game.camera_player_position.y = global_game.player.entity.position.y + global_game.top_left_world_position.y; 
        global_game.camera_player_position.x = global_game.player.entity.position.x;
    } else{
        global_game.camera_player_position = global_game.player.entity.position;
    }


    global_game.input = input;
    global_game.time += delta;
    
    f32 frame_time = 0.0166666f;
    
    f32 real_delta = delta;
    
    delta += previous_delta;
    previous_delta = 0;
    
    if (delta > frame_time){
        global_game.delta = frame_time;
        while(delta > frame_time){
            if (hitstop_countdown > 0){
                hitstop_countdown -= frame_time;
                delta -= 0.001f;
                global_game.delta = 0.001f;
            } else{
                global_game.delta = frame_time;
                delta -= frame_time;
            }
            update(&global_game);
        }
        
        previous_delta = delta;
    }  else{
        if (hitstop_countdown > 0){
            hitstop_countdown -= delta;
            global_game.delta = 0.001f;
        } else{
            global_game.delta = delta;
        }
        update(&global_game);
    }
    
    f32 screen_center = (Buffer->ScreenHeight / unit_size) * 0.5f;
    f32 additional_vertical_position = (input.mouse_screen_position.y / unit_size) - screen_center;
    
    f32 camera_target_y = global_game.camera_player_position.y - ((f32)Buffer->Height / (f32)unit_size) * 0.5f;
    
    camera_target_y += additional_vertical_position * 0.3f;
    
    camera_position.y = lerp(camera_position.y, camera_target_y, global_game.delta * 10);
    loop_world_position(&global_game, &camera_position);

    global_game.delta = real_delta;
    render(&global_game, Buffer);
}

void update(Game *game){
    update_enemies_spawn(game);
    update_player(game);
    update_fly_enemies(game);
    update_fly_enemy_projectiles(game);
    update_particles(game);
    update_camera_shake(game);
    debug_update(game);

    game->player.entity.position.x += game->player.velocity.x * game->delta;
    game->player.entity.position.y += game->player.velocity.y * game->delta;
    
    loop_world_position(game, &game->player.entity.position);
    
    //camera_position = game->player.entity.position;
}

void update_enemies_spawn(Game *game){
    if (current_spawn_index >= SPAWN_COUNT){
        return;
    }

    if (game->time >= spawns[current_spawn_index].spawn_time){
        for (int i = 0; i < spawns[current_spawn_index].fly_enemies_count; i++){
            b32 spawn_up = rnd() % 256 < 128;
            f32 x_position = rnd(0.0f, game->world_size.x);
            f32 y_position = get_camera_position().y + rnd(game->camera_screen_world_size.y * 0.6f,
                                                           game->camera_screen_world_size.y * 1.1f);
            Vector2 position = {x_position, y_position};
            
            add_fly_enemy(game, position);
        }
        current_spawn_index++;
    }
}

void update_player(Game *game){
    Player *player = &game->player;

    f32 vertical = 0;
    if (game->input.up_key) vertical = 1;
    if (game->input.down_key) vertical = -1;
    
    f32 horizontal = 0;
    if (game->input.right_key) horizontal = 1;
    if (game->input.left_key)  horizontal = -1;
    
    if (player->riding_pole){
        horizontal = 0;
    }
    
    f32 wish_speed = horizontal * game->player.base_speed;
    
    accelerate(&(game->player.velocity), game->delta,
               game->player.base_speed, wish_speed, horizontal,
               game->player.grounded ? game->player.acceleration : game->player.air_acceleration);
    
    if (game->player.grounded){
        apply_friction(&(game->player.velocity), game->player.base_speed, game->delta, game->player.friction);
        
        if (abs(player->velocity.x) > 15){
            Vector2 dust_particles_direction = {};
            if (horizontal != 0){
                dust_particles_direction = {-horizontal, 0.5f};
            } else{
                dust_particles_direction = {normalize(player->velocity.x), 0.5f};
            }
            
            f32 speed_progress = abs(game->player.velocity.x) * 0.01f;
            update_overtime_emitter(game, &game->dust_emitter, dust_particles_direction, subtract(player->entity.position, {0, 1}), lerp(1.0f, 10.0f, speed_progress)); 
        }
    }
    
    
    if (!game->player.grounded){
        if (player->velocity.y > player->jump_force * 0.5f){
            player->gravity_multiplier = player->velocity.y / (player->jump_force * 0.5f);
        } else{
            player->gravity_multiplier = 1;
        }
        
        game->player.velocity.y += game->gravity * player->gravity_multiplier * game->delta;
    }
    
    if (game->input.jump_key && game->player.grounded){
        game->player.velocity.y += game->player.jump_force;
        
        game->player.velocity.x += horizontal * game->player.jump_speed_boost;
        
        emit_particles(game, game->dust_emitter, {horizontal, 1}, subtract(player->entity.position, {0, 1}), 3);
    }
    /*
    if (game->player.melee_cooldown_timer > 0){
        game->player.melee_cooldown_timer -= game->delta;
    }
    
    if (game->input.mouse_right_key && player->melee_cooldown_timer <= 0){
        player->melee_cooldown_timer = 0.5f;
        Vector2 direction = subtract(game->input.mouse_world_position, player->entity.position);
        emit_particles(game, player->cleaning_emitter, direction, player->entity.position, 1.0f);
    }
    */
    //shoot logic
    Player::shooter *shoot = &player->shoot;
    Vector2 player_to_mouse = subtract(game->input.mouse_world_position, game->camera_player_position);
    normalize(&player_to_mouse);

    if (shoot->just_shoot){
        shoot->just_shoot = 0;
    }
    
    if (shoot->cooldown_timer > 0){
        shoot->cooldown_timer -= game->delta;
    }
    
    //charging
    if (game->input.mouse_left_key || game->input.mouse_right_key){
        shoot->holding_shot     = game->input.mouse_left_key;
        shoot->holding_cleaning = game->input.mouse_right_key;
        shoot->holding_timer += game->delta;
        
        if (shoot->holding_timer >= shoot->perfect_hold_time && !shoot->perfect_charged){
            shoot->perfect_charged = 1;
            
            emit_particles(game, game->player.charged_emitter, player_to_mouse, add(player->entity.position, player_to_mouse), 1.0f);
            
        }
        //shooting
    } else if ((shoot->holding_shot || shoot->holding_cleaning) && player->shoot.cooldown_timer <= 0){
        b32 shooting = shoot->holding_shot;
        b32 cleaning = shoot->holding_cleaning;
    
        shoot->holding_shot = 0;
        shoot->holding_cleaning = 0;
        shoot->perfect_charged = 0;
        
        f32 current_perfect_buffer = shooting ? shoot->perfect_buffer : (shoot->perfect_buffer * 0.25f);
        
        b32 perfect_shoot = shoot->holding_timer >= shoot->perfect_hold_time - 0.05f
                         && shoot->holding_timer <= (shoot->perfect_hold_time + current_perfect_buffer);
                         
        shoot->just_shoot = perfect_shoot ? 2 : 1;
        player->shoot.cooldown_timer = perfect_shoot ? 0.1f : player->shoot.cooldown;
        
        shoot->holding_timer = 0;
        
        //recoil
        Vector2 recoil_direction = multiply(player_to_mouse, -1);
        
        if (shooting){
            if (player->velocity.x * recoil_direction.x < 0){
                player->velocity.x = 0;
            }
            if (player->velocity.y * recoil_direction.y < 0){
                player->velocity.y = 0;
            }
        }
        add(&player->velocity, multiply(recoil_direction, perfect_shoot ? shoot-> push_force : shoot->push_force * 0.3f));

        
        if (shooting){
            shoot_rifle(game, player, shoot, player_to_mouse, perfect_shoot);
        } else{
            emit_particles(game, player->cleaning_emitter, player_to_mouse, player->entity.position, perfect_shoot ? 1.0f : 0.05f);
        }
    } else{
        shoot->holding_timer = 0;
    }
    
    clamp(&player->velocity.y, -150, 200);
    
    check_player_collisions(game);
    //printf("%d\n", (int)game->player.velocity.x);
}

void shoot_rifle(Game *game, Player *player, Player::shooter *shoot, Vector2 player_to_mouse, b32 perfect_shoot){
    f32 shoot_length = perfect_shoot ? 250 : 40;
    
    Vector2 end_point = add(player->entity.position, multiply(player_to_mouse, shoot_length));
    Line line = {};
    line.start_position = player->entity.position;
    line.end_position   = end_point;
    line.start_width = perfect_shoot ? 5.0f : 2.0f;
    line.end_width = line.start_width;
    
    shake_camera(game, perfect_shoot ? 0.5f : 0.1f);
    
    Vector2 bullet_shell_direction = player_to_mouse;
    clamp(&bullet_shell_direction.y, 0.2f, 1.0f);
    bullet_shell_direction.x = -player_to_mouse.x;
    emit_particles(game, game->player.shoot_emitter, bullet_shell_direction, game->player.entity.position, 1.0f);
    
    //shoot line render
    line_entity visual_line = {};
    visual_line.line = line;
    visual_line.line.start_width = perfect_shoot ? 2.0f : 1.0f;
    visual_line.line.end_width = 0.5f;
    visual_line.color = perfect_shoot ? 0xbf212f : 0xf9a73e;
    visual_line.max_lifetime = perfect_shoot ? 0.4f : 0.2f;
    
    array_add(&game->line_entities, &visual_line);
    
    //@TODO: Free this hits
    line_hits hits = check_line_collision(game, line);
    int collision_count = hits.enter_count >= hits.exit_count ? hits.enter_count : hits.exit_count;
    
    for (int i = 0; i < collision_count; i++){
        particle_emitter *hit_emitter;
        if (hits.hit_types[i] == 2){
            hit_emitter = &game->blood_emitter;
        } else{
            hit_emitter = &player->shoot.wall_hit_emitter;
        }
    
        if (hits.enter_count > i){
            emit_particles(game, *hit_emitter, multiply(player_to_mouse, -1), hits.enter_positions[i], 0.5f);
        }
        if (hits.exit_count > i){
            emit_particles(game, *hit_emitter, player_to_mouse, hits.exit_positions[i], 1.0f);
        }
        
        if (hits.enter_count > i && hits.exit_count > i){
            //bullet line in tiles
            Line wall_line = {};
            wall_line.start_position = hits.enter_positions[i];
            wall_line.end_position   = hits.exit_positions[i];
            wall_line.start_width = 0.01f;
            wall_line.end_width = 0.01f;
            
            line_entity wall_visual_line = {};
            wall_visual_line.line = wall_line;
            wall_visual_line.max_lifetime = 0;
            
            if (hits.hit_types[i] == 1){
                wall_visual_line.color = 0xffffff;
                array_add(&game->line_entities, &wall_visual_line);
            } else if (hits.hit_types[i] == 2){
                //hit enemy
                wall_visual_line.color = 0xcc3333;
                subtract(&wall_visual_line.line.start_position, hits.enemy_hits[i]->entity.position);
                subtract(&wall_visual_line.line.end_position, hits.enemy_hits[i]->entity.position);
                array_add(&hits.enemy_hits[i]->lines, &wall_visual_line);
                
                hits.enemy_hits[i]->hp -= perfect_shoot ? shoot->pefrect_damage : shoot->damage;
                
                hitstop_countdown += 0.1f;
                shake_camera(game, 0.1f);
            }
        }
    }
}


void update_fly_enemies(Game *game){
    for (int i = 0; i < game->fly_enemies.count; i++){
        fly_enemy *fly = (fly_enemy *)array_get(&game->fly_enemies, i);
        
        if (fly->hp <= 0){
            //array_remove(&game->fly_enemies, i);
            continue;
        }
        
        Vector2 vector_to_player = subtract(game->player.entity.position, fly->entity.position);
        Vector2 direction_to_player = normalized(vector_to_player);
        f32 distance_to_player = magnitude(vector_to_player);
        
        if (in_blood(fly->entity)){
            fly->time_in_blood += game->delta;
            fly->in_blood_progress = clamp01(fly->time_in_blood / fly->max_time_in_blood);
        }
        
        //blood progress multipliers
        update_overtime_emitter(game, &game->blood_emitter, {0, 1}, fly->entity.position, fly->in_blood_progress);
        fly->charge_duration = lerp(fly->start_charge_duration, fly->start_charge_duration * 0.5f, fly->in_blood_progress);
        fly->strafe_duration = lerp(fly->start_strafe_duration, fly->start_strafe_duration * 0.5f, fly->in_blood_progress);
        fly->circle_speed = lerp(fly->start_circle_speed, fly->start_circle_speed * 3.0f, fly->in_blood_progress);
        
        if (fly->charging){
            fly->charge_timer += game->delta;
            
            f32 charge_progress = fly->charge_timer / fly->charge_duration;
            clamp(&charge_progress, 0.0f, 1.0f);
            
            line_entity charge_line = {};
            charge_line.line = {};
            charge_line.line.start_position = fly->entity.position;
            
            if (charge_progress < 0.8f){
                fly->charge_end_position = add(fly->entity.position, multiply(direction_to_player, distance_to_player * 1.3f));
            }
            
            charge_line.line.end_position = fly->charge_end_position;
            f32 width = lerp(0.0f, 4.0f, EaseInCirc(charge_progress));
            charge_line.line.start_width = width;
            charge_line.line.end_width = width;
            
            charge_line.visual_start_width = width;
            charge_line.visual_end_width = width;
            charge_line.die_after_drawing = 1;
            charge_line.color = lerp_color(0x2244aa, 0xaa2244, EaseInCirc(charge_progress));
            
            array_add(&game->line_entities, &charge_line);
            
            if (charge_progress >= 1){
                fly->charge_timer = 0;
                fly->charging = 0;
                fly->strafing = 1;
                
                fly->strafe_start_position = fly->entity.position;
                fly->strafe_target_position = fly->charge_end_position;
                fly->picked_strafe_position = 1;
            }
        }

        if (fly->strafing && !fly->picked_strafe_position){
            if (distance_to_player < 20){
                fly->strafing = 0;
                fly->circling = 1;
            }
            
            if (fly->strafing){
                Vector2 strafe_direction = 
                        {fly->strafe_count & 1 ? -1.0f : 1.0f,
                         normalize(direction_to_player.y) * 1};
                normalize(&strafe_direction);
            
                fly->strafe_start_position = fly->entity.position;
                fly->strafe_target_position = add(fly->entity.position, multiply(strafe_direction, fly->strafe_distance));
                fly->picked_strafe_position = 1;
            }
        }

        
        if (fly->strafing){
            fly->strafe_t += game->delta / fly->strafe_duration;
            
            fly->entity.position = lerp(fly->strafe_start_position, fly->strafe_target_position, EaseOutElastic(fly->strafe_t));
            
            if (fly->strafe_t >= 1){
                fly->strafe_t = 0;
                fly->picked_strafe_position = 0;
                fly->strafe_count++;
                
                if (fly->strafe_count >= fly->max_strafe_count){
                    fly->strafe_count = 0;
                    fly->strafing = 0;
                    fly->circling = 1;
                }
            }
        }
        
        if (fly->circling){
            if (fly->circling_time <= 0){
                fly->circle_origin = fly->entity.position;
            }
        
            fly->circling_time += game->delta;
        
            fly->circle_angle += game->delta * fly->circle_speed;
            fly->entity.position.x = lerp(fly->entity.position.x,
                                          fly->circle_origin.x + fly->circle_radius * cos(fly->circle_angle),
                                          game->delta * 10);
            fly->entity.position.y = lerp(fly->entity.position.y,
                                          fly->circle_origin.y + fly->circle_radius * sin(fly->circle_angle),
                                          game->delta * 10);
                                          
            if (fly->circling_time > 2){
                fly->circling_time = 0;
                fly->circling = 0;
                
                u32 chance = rnd() % 256;
                b32 will_shoot = chance < 190;
                
                if (will_shoot){
                    fly->strafing = 1;
                    
                    spawn_fly_enemy_projectile(game, direction_to_player, fly->entity.position);
                } else{
                    fly->charging = 1;
                }
            }
        }
        
        if (check_box_collision(&fly->entity, &game->player.entity)){
            //Enemy hit player
            game->player.velocity.y += 60;
            emit_particles(game, game->blood_emitter, direction_to_player, game->player.entity.position, 1);
            shake_camera(game, 0.6f);
            
            game->color_change_countdown = 0.05f;
            game->current_color_palette = 0xFF8F8F;
            
            hitstop_countdown += 0.2f;
            clamp(&hitstop_countdown, 0, 1);
        }
        
        loop_world_position(game, &fly->entity.position);
    }
}

void update_fly_enemy_projectiles(Game *game){
    for (int i = 0; i < game->fly_enemy_projectiles.count; i++){
        fly_enemy_projectile *projectile = (fly_enemy_projectile *)array_get(&game->fly_enemy_projectiles, i);
        
        projectile->lifetime += game->delta;
        
        if (projectile->lifetime > 10){
            array_remove(&game->fly_enemy_projectiles, i);
            return;
        }
        
        Vector2 start_velocity = multiply(projectile->direction, 10.0f);
        Vector2 end_velocity = multiply(projectile->direction, projectile->target_speed);
        
        f32 speed_progress = projectile->lifetime / projectile->time_for_max_speed;
        clamp(&speed_progress, 0, 1);
        projectile->velocity = lerp(start_velocity, end_velocity, speed_progress);
        
        add(&projectile->entity.position, multiply(projectile->velocity, game->delta));
        loop_world_position(game, &projectile->entity.position);
        
        if (check_box_collision(&projectile->entity, &game->player.entity)){
            //projectile hit player
            game->player.velocity.y += 40;
            
            emit_particles(game, game->blood_emitter, projectile->direction, game->player.entity.position, 1);
            shake_camera(game, 0.6f);
            
            game->color_change_countdown = 0.05f;
            game->current_color_palette = 0xFF8F8F;

            hitstop_countdown += 0.2f;
            
            array_remove(&game->fly_enemy_projectiles, i);
            return;
        }
        
        projectile->line_render.line.start_position = projectile->entity.position;
        Vector2 line_end_position = add(projectile->entity.position, multiply(projectile->direction, lerp(1.0f, 6.0f, speed_progress * speed_progress)));
        projectile->line_render.line.end_position = line_end_position;
        
        projectile->line_render.visual_start_width = lerp(4.0f, 2.0f, speed_progress * speed_progress);
        projectile->line_render.visual_end_width = lerp(2.5f, 1.0f, speed_progress * speed_progress);
    }
}

void spawn_fly_enemy_projectile(Game *game, Vector2 direction, Vector2 start_position){
    fly_enemy_projectile projectile = {};
    projectile.entity = {};
    projectile.line_render = {};
    
    projectile.entity.position = start_position;
    projectile.entity.scale = {2, 2};
    
    projectile.line_render.line = {};
    projectile.line_render.line.start_width = 2.0f;
    projectile.line_render.line.end_width = 1.7f;
    projectile.line_render.color = 0xff6677;
    
    projectile.line_render.visual_start_width = 3.5f;
    projectile.line_render.visual_end_width = 2.0f;
    
    projectile.direction = direction;
    projectile.target_speed = 140;
    projectile.time_for_max_speed = 3.0f;
    
    
    array_add(&game->fly_enemy_projectiles, &projectile);
}


void accelerate(Vector2 *velocity, f32 delta, f32 base_speed, f32 wish_speed, int direction, f32 acceleration){
    if (direction == 0) return;
    f32 new_speed = velocity->x + acceleration * direction * delta;
    
    if (abs(new_speed) > base_speed && new_speed * direction > 0){
        new_speed = velocity->x;
    }
    
    velocity->x = new_speed;
}
void apply_friction(Vector2 *velocity, f32 max_speed, f32 delta, f32 friction){
    if (abs(velocity->x) > max_speed){
        friction *= 2 + abs(velocity->x) / max_speed;
    }
    f32 friction_force = friction * -normalize(velocity->x) * delta;
    velocity->x += friction_force;
}

void add_fly_enemy(Game *game, Vector2 position){
    loop_world_position(game, &position);

    fly_enemy enemy = {};
    enemy.entity.position = position;
    enemy.entity.scale = {4, 5};
    
    enemy.lines = array_init(sizeof(line_entity), 100);
    
    line_entity triangle_line_1 = {};
    triangle_line_1.line = {};
    triangle_line_1.line.start_position = {-2, -2};
    triangle_line_1.line.end_position = {0, 3};
    triangle_line_1.visual_start_width = 1.5f;
    triangle_line_1.visual_end_width = 0.7f;
    line_entity triangle_line_2 = {};
    triangle_line_2.line = {};
    triangle_line_2.line.start_position = {2, -1};
    triangle_line_2.line.end_position = {0, 3};
    triangle_line_2.visual_start_width = 1.5f;
    triangle_line_2.visual_end_width = 0.7f;
    line_entity triangle_line_3 = {};
    triangle_line_3.line = {};
    triangle_line_3.line.start_position = {-2, -2};
    triangle_line_3.line.end_position = {0, -1};
    triangle_line_3.visual_start_width = 1.5f;
    triangle_line_3.visual_end_width = 0.5f;
    line_entity triangle_line_4 = {};
    triangle_line_4.line = {};
    triangle_line_4.line.start_position = {2, -2};
    triangle_line_4.line.end_position = {0, -1};
    triangle_line_4.visual_start_width = 1.5f;
    triangle_line_4.visual_end_width = 0.5f;
    line_entity left_wing_line = {};
    left_wing_line.line = {};
    left_wing_line.line.start_position = {0, 0};
    left_wing_line.line.end_position = {-4, 4};
    left_wing_line.visual_start_width = 0.3f;
    left_wing_line.visual_end_width = 2;
    line_entity right_wing_line = {};
    right_wing_line.line = {};
    right_wing_line.line.start_position = {0, 0};
    right_wing_line.line.end_position = {4, 4};
    right_wing_line.visual_start_width = 0.3f;
    right_wing_line.visual_end_width = 2;
    
    array_add(&enemy.lines, &triangle_line_1);
    array_add(&enemy.lines, &triangle_line_2);
    array_add(&enemy.lines, &triangle_line_3);
    array_add(&enemy.lines, &triangle_line_4);
    array_add(&enemy.lines, &left_wing_line);
    array_add(&enemy.lines, &right_wing_line);
    
    array_add(&global_game.fly_enemies, &enemy);
}


void check_player_collisions(Game *game){
    game->player.grounded = 0;
    
    calculate_player_tilemap_collisions(game, check_tilemap_collisions(game, game->player.velocity, game->player.entity));
    
    if (in_blood(game->player.entity)){
        game->player.in_blood_time += game->delta;
        
        game->player.not_in_blood_time = 0;
        //shake_camera(game, game->delta);
        game->im_dying_man = 1;
    } else{
        game->player.not_in_blood_time += game->delta;
        game->im_dying_man = 0;
        if (game->player.not_in_blood_time > 5){
            game->player.in_blood_time -= game->delta;
        }
    }
    
    clamp(&game->player.in_blood_time, 0, game->player.max_in_blood_time);
    
    game->player.in_blood_progress = game->player.in_blood_time / game->player.max_in_blood_time; 
}

void calculate_player_tilemap_collisions(Game *game, collision *collisions_data){
    Player *player = &game->player;
    
    player->riding_pole = 0;
    
    for (int i = 0; i < collisions_count; i++){
        if (!collisions_data[i].collided) break;
        switch (collisions_data[i].tile_type){
            case Ground:{
                if (collisions_data[i].normal.y > 0){
                    player->grounded = 1;
                }
                
                f32 bounce_power = magnitude(multiply(player->velocity, collisions_data[i].normal));
                if (collisions_data[i].normal.y != 1){
                    bounce_power *= 1.2f;
                }
                
                if (bounce_power > 20){
                    f32 t = (bounce_power - 20) / 150.0f;
                    emit_particles(game, game->dust_emitter, collisions_data[i].normal, subtract(player->entity.position, collisions_data[i].normal), lerp(0.01f, 8.0f, t * t * t));
                    
                    shake_camera(game, lerp(0.0f, 0.6f, t * t));
                }
                
                add(&player->velocity, multiply(collisions_data[i].normal, bounce_power));
            } break;
            
            case Pole:{
                if (game->input.up_key){
                    player->velocity.y += 100 * game->delta;
                    player->velocity.x *= 1.0f - game->delta * 3;
                    player->riding_pole = 1;
                    shake_camera(game, game->delta * 0.3f);
                    
                    update_overtime_emitter(game, &player->pole_ride_emitter, {0, -1}, collisions_data[i].obstacle_position, 1);
                }
            } break;
        }
    }
    
    free(collisions_data);
}

void update_particles(Game *game){
    for (int i = 0; i < game->particles.count; i++){
        Particle *particle = ((Particle *)array_get(&game->particles, i));
        particle->lifetime += game->delta;
        
        if (particle->lifetime >= particle->max_lifetime){
            array_remove(&game->particles, i);
            continue;
        }
        
        f32 t_lifetime = particle->lifetime / particle->max_lifetime;
        particle->entity.scale = lerp(particle->original_scale, Vector2_zero, t_lifetime * t_lifetime);
        
        particle->velocity.y += game->gravity * game->delta;
        
        if (particle->colliding){
            calculate_particle_tilemap_collisions(game, particle, check_tilemap_collisions(game, particle->velocity, particle->entity));
        }
        
        Vector2 next_position = add(particle->entity.position, multiply(particle->velocity, game->delta));
        
        particle->entity.position = next_position;
        
        loop_world_position(game, &particle->entity.position);
        
        if (particle->leave_splash){
            add_splash(game, particle->entity, particle-> splash_color == 0 ? particle->color : particle->splash_color);
        }
    }
}

void calculate_particle_tilemap_collisions(Game *game, Particle *particle, collision *collisions_data){
    for (int i = 0; i < collisions_count; i++){
        if (!collisions_data[i].collided) break;
        switch (collisions_data[i].tile_type){
            case Ground:{
                if (collisions_data[i].normal.y > 0){
                    apply_friction(&particle->velocity, 50, game->delta, game->player.friction * 0.7f);
                }
                
                f32 particle_magnitude = magnitude(multiply(particle->velocity, collisions_data[i].normal));
                
                Vector2 added_velocity = multiply(collisions_data[i].normal,  particle_magnitude);
                
                added_velocity = multiply(added_velocity, 1.2f);
                
                add(&particle->velocity, added_velocity);
            } break;
        }
    }
    
    free(collisions_data);
}

Vector2 get_tile_world_position(Game *game, int x, int y){
    return {(f32)x * game->tilemap.block_scale, ((f32)game->tilemap.rows - y - 1) * game->tilemap.block_scale};
}

collision *check_tilemap_collisions(Game *game, Vector2 velocity, Entity entity){ 
    collision *collisions_data = (collision *)malloc(collisions_count * sizeof(collision));
    for (int i = 0; i < collisions_count; i++){
        collisions_data[i].collided = 0;
    }
    int collided_count = 0;

    Entity vertical_future_entity = entity;        
    vertical_future_entity.position = add(entity.position, {0, velocity.y * game->delta});
    
    Entity horizontal_future_entity = entity;        
    horizontal_future_entity.position = add(entity.position, {velocity.x * game->delta, 0});
            
    for (int y = 0; y < game->tilemap.rows; y++){
        for (int x = 0; x < game->tilemap.columns; x++){
            if (collided_count >= collisions_count) break;
            if (level1[y][x] == 0) continue;
            Entity tile_entity = {};
            tile_entity.position = get_tile_world_position(game, x, y); 
            tile_entity.scale = {(f32)game->tilemap.block_scale, (f32)game->tilemap.block_scale};
            
            if (check_box_collision(&vertical_future_entity, &tile_entity)){
                collisions_data[collided_count].normal = {0, -normalize(velocity.y)};
                collisions_data[collided_count].tile_type = (TileType)level1[y][x];
                collisions_data[collided_count].collided = 1;
                collisions_data[collided_count].obstacle_position = tile_entity.position;
                collided_count++;
            }
            if (velocity.x != 0 && check_box_collision(&horizontal_future_entity, &tile_entity)){
                collisions_data[collided_count].normal = {-normalize(velocity.x), 0};
                collisions_data[collided_count].tile_type = (TileType)level1[y][x];
                collisions_data[collided_count].collided = 1;
                collisions_data[collided_count].obstacle_position = tile_entity.position;
                collided_count++;
            }
        }
    }
    
    return collisions_data;
}

fly_enemy *check_enemy_collision(Game *game, Entity entity){
    for (int i = 0; i < game->fly_enemies.count; i++){
        fly_enemy *enemy = (fly_enemy *)array_get(&game->fly_enemies, i);
        
        if (check_box_collision(&entity, &enemy->entity)){
            return enemy;
        }
    }
    
    return 0;
}

collision check_particles_collisions(Game *game, Vector2 velocity, Entity entity){
    Entity vertical_future_entity = entity;        
    vertical_future_entity.position = add(entity.position, {0, velocity.y * game->delta});
    
    Entity horizontal_future_entity = entity;        
    horizontal_future_entity.position = add(entity.position, {velocity.x * game->delta, 0});
    
    collision collision_data = {};
    collision_data.collided = 0;

    for (int i = 0; i < game->particles.count; i++){
        Particle *particle = (Particle *)array_get(&game->particles, i);
        if (particle->lifetime < 1) {
            continue;
        }
        if (check_box_collision(&vertical_future_entity, &(particle->entity))){
            collision_data.normal   = {0, -normalize(velocity.y)};
            collision_data.obstacle_velocity =  &particle->velocity;
            collision_data.collided = 1;
            return collision_data;            
        }
        
        if (check_box_collision(&horizontal_future_entity, &(particle->entity))){
            
            collision_data.normal   = {-normalize(velocity.x), 0};
            collision_data.obstacle_velocity =  &particle->velocity;
            collision_data.collided = 1;
            return collision_data;            
        }
    }

    return collision_data;
}

b32 check_box_collision(Entity *rect1, Entity *rect2){
    f32 rect1X = rect1->position.x;
    f32 rect1Y = rect1->position.y;
    f32 rect2X = rect2->position.x;
    f32 rect2Y = rect2->position.y;
    
    f32 rect1W = rect1->scale.x;
    f32 rect1H = rect1->scale.y;
    f32 rect2W = rect2->scale.x;
    f32 rect2H = rect2->scale.y;
    
    b32 solution = rect1X + rect1W * 0.5f > rect2X - rect2W * 0.5f &&
                   rect1X - rect1W * 0.5f < rect2X + rect2W * 0.5f &&
                   rect1Y + rect1H * 0.5f > rect2Y - rect2H * 0.5f &&
                   rect1Y - rect1H * 0.5f < rect2Y + rect2H * 0.5f;
    /*
    if (rect1X < rect2X + rect2W &&
        rect2X < rect1X + rect1W &&
        rect1Y < rect2Y + rect2H &&
        rect2Y < rect1Y + rect1H)
    {
    */
    
    return solution;
}

line_hits check_line_collision(Game *game, Line line){
    Vector2 vector_to_end = subtract(line.end_position, line.start_position);
    int points_count = (((int)magnitude(vector_to_end) + 10) * 4);
    
    Vector2 direction = normalized(vector_to_end);
    
    b32 in_some_collider = 0;
    
    line_hits hits = {};
    hits.enter_positions = (Vector2 *)malloc(collisions_count * sizeof(Vector2));
    hits.exit_positions  = (Vector2 *)malloc(collisions_count * sizeof(Vector2));
    hits.hit_types  = (i32 *)malloc(collisions_count * sizeof(i32));
    hits.enemy_hits  = (fly_enemy **)malloc(collisions_count * sizeof(fly_enemy *));
    
    for (int i = 0; i <= points_count; i++){
        b32 hit_anything = 0;
    
        f32 t = lerp((f32)0, (f32)1, (f32)i / (f32)points_count);
        
        f32 point_x = lerp(line.start_position.x, line.end_position.x, t);
        f32 point_y = lerp(line.start_position.y, line.end_position.y, t);
        
        Entity line_point_entity = {};
        line_point_entity.position = {point_x, point_y};
        line_point_entity.scale = {line.start_width, line.start_width};
        loop_world_position(game, &line_point_entity.position);
        
        fly_enemy *enemy_collision = check_enemy_collision(game, line_point_entity);
        if (enemy_collision){
            if (!in_some_collider){
                hits.enter_positions[hits.enter_count] = subtract(line_point_entity.position, multiply(direction, 0.2f));
                hits.hit_types[hits.enter_count] = 2;
                hits.enemy_hits[hits.enter_count] = enemy_collision;
                hits.enter_count++;
            }
            
            in_some_collider = 1;
            hit_anything = 1;
        }
                        
        for (int y = 0; y < game->tilemap.rows && !hit_anything; y++){
            for (int x = 0; x < game->tilemap.columns && !hit_anything; x++){
                if (hits.enter_count >= collisions_count || hits.exit_count >= collisions_count){
                    continue;
                }
            
                if (level1[y][x] == 0 || level1[y][x] == 2) continue;
                
                Entity tile_entity = {};
                tile_entity.position = get_tile_world_position(game, x, y);
                tile_entity.scale = {(f32)game->tilemap.block_scale, (f32)game->tilemap.block_scale};
                
                if (check_box_collision(&line_point_entity, &tile_entity)){
                    if (!in_some_collider){
                        hits.enter_positions[hits.enter_count] = subtract(line_point_entity.position, multiply(direction, 0.2f));
                        hits.hit_types[hits.enter_count] = 1;
                        
                        hits.enter_count++;
                    }
                    
                    in_some_collider = 1;
                    hit_anything = 1;
                } 
            }
        }
        
        if (hits.exit_count < collisions_count && !hit_anything && in_some_collider){
            hits.exit_positions[hits.exit_count] = add(line_point_entity.position, multiply(direction, 0.2f));
            hits.exit_count++;
            
            in_some_collider = 0;
        }
    }
    
    return hits;
}

void render(Game *game, screen_buffer *Buffer){
    if (game->color_change_countdown > 0){
        game->color_change_countdown -= game->delta;
        if (game->color_change_countdown <= 0){
            if (game->current_color_palette == 0xFF8F8F){
                game->current_color_palette = 0x8FFF8F;
                game->color_change_countdown = 0.05f;
            } else if (game->current_color_palette == 0x8FFF8F){
                game->current_color_palette = 0x8F8FFF;
                game->color_change_countdown = 0.05f;
            } else{
                game->current_color_palette = 0xFFFFFF;
            }
        }
    }

    fill_background(game, Buffer, game->background_gradient);
    
    draw_entities(game, Buffer);

    Player *player = &game->player;
    draw_rect(Buffer, player->entity.position, player->entity.scale, 0x11ff3423);
    //Vector2 added = add(game->player.entity.position, {10, 10});
    
    Vector2 player_to_mouse = subtract(game->input.mouse_world_position, game->camera_player_position);
    normalize(&player_to_mouse);
    
    //Draw Rifle
    local_persist f32 length_multiplier = 1.0f; 
    Player::shooter *shoot = &player->shoot;
    if (shoot->holding_shot || shoot->holding_cleaning){
        f32 perfect_length_multiplier = 2.0f;
        f32 max_length_multiplier = 2.2f;
        
        f32 start_length_multiplier = length_multiplier < 1.0f ? length_multiplier : 1.0f;
        
        if (shoot->holding_timer <= shoot->perfect_hold_time){
            f32 t = shoot->holding_timer / shoot->perfect_hold_time;
            length_multiplier = lerp(start_length_multiplier, perfect_length_multiplier, t * t);
        } else if (shoot->holding_timer <= shoot->perfect_hold_time + shoot->perfect_buffer){
            f32 t = (shoot->holding_timer - shoot->perfect_hold_time) / shoot->perfect_buffer;
            length_multiplier = lerp(perfect_length_multiplier, max_length_multiplier, t);
        } else{
            length_multiplier = lerp(length_multiplier, 1.0f, game->delta * 4.0f);
        }
    } else if (shoot->just_shoot){
        length_multiplier = shoot->just_shoot == 2 ? 0.25f : 0.7f;
    } else{
        length_multiplier = lerp(length_multiplier, 1.0f, game->delta * 4.0f);
    }
    
    f32 base_length_multiplier   = 1.0f;
    f32 middle_length_multiplier = 1.0f;
    f32 end_length_multiplier    = 1.0f;

    if (length_multiplier > 1.0f){
        base_length_multiplier   = lerp(length_multiplier, length_multiplier * 1.5f, game->delta * 20);
        middle_length_multiplier = lerp(length_multiplier, length_multiplier * 1.5f, game->delta * 20);
        end_length_multiplier    = lerp(length_multiplier, length_multiplier * 1.5f, game->delta * 20);
    } else if (length_multiplier < 1.0f){
        base_length_multiplier   = length_multiplier;
        middle_length_multiplier = length_multiplier;
        end_length_multiplier    = length_multiplier;
    }
    
    draw_line(Buffer,
              subtract(player->entity.position, multiply(player_to_mouse, 0.2f)),
              add(player->entity.position, multiply(player_to_mouse, 1.0f * base_length_multiplier)),
              1.7f, 1.7f, 0x555555);
    draw_line(Buffer,
              add(player->entity.position, multiply(player_to_mouse, 1.5f * length_multiplier)),
              add(player->entity.position, multiply(player_to_mouse, 2.5f * middle_length_multiplier)),
              1.3f, 1.3f, 0x777777);
    draw_line(Buffer,
              add(player->entity.position, multiply(player_to_mouse, 3.0f * length_multiplier)),
              add(player->entity.position, multiply(player_to_mouse, 4.5f * end_length_multiplier)),
              0.7f, 0.6f, 0x999999);
    
    //draw_line(Buffer, game->player.entity.position, game->input.mouse_world_position, 0.2f, 0xff5533);
    
    //draw_rect(Buffer, game->input.mouse_world_position.x, game->input.mouse_world_position.y, 3, 3, 0xbc32fd);
    //draw_rect(Buffer, player.entity.position.x, player.entity.position.y, player.scale.x * unit_size, player.scale.y * unit_size, 0xff5533);
}

void draw_entities(Game *game, screen_buffer *Buffer){
    for (int y = 0; y < game->tilemap.rows; y++){
        for (int x = 0; x < game->tilemap.columns; x++){
            Entity tile_entity = {};
            tile_entity.position = get_tile_world_position(game, x, y);
            tile_entity.scale = {(f32)game->tilemap.block_scale, (f32)game->tilemap.block_scale};
            
            //i32 position_y = game->tilemap.rows - y - 1;
            //i32 position_x = x;

            switch ((TileType)level1[y][x]){
                case None:{
                    continue;
                } break;
                case Ground:{
                    draw_world_rect(game, Buffer, tile_entity.position, tile_entity.scale, game->tiles_gradient);
                } break;
                case Pole:{
                    tile_entity.scale.x *= 0.2f;
                    draw_world_rect(game, Buffer, tile_entity.position, tile_entity.scale, game->pole_gradient);
                } break;
            }
            
        }
    }
    
    draw_splashes(game, Buffer);
    
    //particles
    for (int i = 0; i < game->particles.count; i++){
        Particle *particle = ((Particle *)array_get(&game->particles, i));
        
        if (particle->shape == (particle_shape)0){
            draw_rect(Buffer, particle->entity.position, particle->entity.scale, particle->color);
        } else if (particle->shape == (particle_shape)1){
            Vector2 end_position = add(particle->entity.position, multiply(particle->velocity, 0.1f));
            draw_line(Buffer, particle->entity.position, end_position, particle->entity.scale.x, particle->entity.scale.x, particle->color);
        }
    }
    
    //line entities
    for (int i = 0; i < game->line_entities.count; i++){
        line_entity *line = (line_entity *)array_get(&game->line_entities, i);
        
        line->lifetime += game->delta;
        
        if (line->max_lifetime > 0 && line->lifetime >= line->max_lifetime){
            array_remove(&game->line_entities, i);
            continue;
        }
        
        if (line->max_lifetime > 0){
            f32 time_to_max = line->max_lifetime * 0.4f;
            
            if (line->lifetime <= time_to_max){
                f32 t = line->lifetime / time_to_max;
                line->visual_start_width = lerp(0.0f, line->line.start_width, EaseOutQuint(t));
                line->visual_end_width = lerp(0.0f, line->line.end_width, EaseOutQuint(t));
            } else{
                f32 t = (line->lifetime - time_to_max) / (line->max_lifetime - time_to_max);
                line->visual_start_width = lerp(line->line.start_width, 0.0f, EaseInCirc(t));
                line->visual_end_width = lerp(line->line.end_width, 0.0f, EaseInCirc(t));
            }
        } else{
            line->visual_start_width = line->line.start_width;
            line->visual_end_width = line->line.end_width;
        }
        
        if (line->color != 0xffffff){
            draw_line(Buffer, line->line.start_position, line->line.end_position, line->visual_start_width, line->visual_end_width, line->color);
        } else{
            draw_line_gradient(game, Buffer, line->line.start_position.x, line->line.start_position.y, line->line.end_position.x, line->line.end_position.y, line->visual_start_width, line->visual_end_width, game->background_gradient);
        
        }
        
        if (line->die_after_drawing){
            array_remove(&game->line_entities, i);
        }
    }
    
    //draw enemies
    for  (int i = 0; i < game->fly_enemies.count; i++){ 
        fly_enemy *enemy = (fly_enemy *)array_get(&game->fly_enemies, i);
        
        //hit box
        //draw_rect(Buffer, enemy->entity.position, enemy->entity.scale, 0xaa3344);
        b32 offsetting_on_death;
        if (enemy->hp <= 0 && !enemy->render_dead){
            enemy->render_dead = 1;
            offsetting_on_death = 1;
        }
        
        for (int j = 0; j < enemy->lines.count; j++){
            line_entity *line_arr = (line_entity *)array_get(&enemy->lines, j);
            Vector2 start_position = add(line_arr->line.start_position, enemy->entity.position);
            Vector2 end_position = add(line_arr->line.end_position, enemy->entity.position);

            if (offsetting_on_death){
                start_position.x += rnd(-3.5f, 3.5f);
                start_position.y += rnd(-3.5f, 3.5f);
                end_position.x   += rnd(-3.5f, 3.5f);
                end_position.y   += rnd(-3.5f, 3.5f);
                line_arr->line.start_position = subtract(enemy->entity.position, start_position);
                line_arr->line.end_position   = subtract(enemy->entity.position, end_position);
            }
            
            if (enemy->hp > 0){
                line_entity line = *line_arr;
                //f32 random_multiplier = ((f32)(rand() % 1000) * 0.0001f) - 0.5f;
                start_position.x += rnd(-0.5f, 0.5f);
                start_position.y += rnd(-0.5f, 0.5f);
                end_position.x   += rnd(-0.5f, 0.5f);
                end_position.y   += rnd(-0.5f, 0.5f);
                
                draw_line(Buffer, start_position, end_position, line.visual_start_width, line.visual_end_width, lerp_color(0x010101, 0xff1111, enemy->in_blood_progress));
            } else{
                draw_line_gradient(game, Buffer, start_position, end_position, line_arr->visual_start_width, line_arr->visual_end_width, game->darker_background_gradient);
            }
        }
        
        offsetting_on_death = 0;
    }
    
    //draw fly enemy projectiles
    for  (int i = 0; i < game->fly_enemy_projectiles.count; i++){ 
        fly_enemy_projectile *projectile = (fly_enemy_projectile *)array_get(&game->fly_enemy_projectiles, i);
        
        draw_line(Buffer, projectile->line_render);
    }

}

void emit_particles(Game *game, particle_emitter emitter, Vector2 direction, Vector2 start_position, f32 count_multiplier){
    normalize(&direction);
    rnd_state++;
    int count = rnd((int)emitter.count_min, (int)emitter.count_max);
    count *= count_multiplier; 
    
    for (int i = 0; i < count; i++){
        shoot_particle(game, emitter, direction, start_position);
    }
}

void update_overtime_emitter(Game *game, particle_emitter *emitter, Vector2 direction, Vector2 start_position, f32 count_multiplier){
    emitter->emitting_timer += game->delta;
    f32 emit_delay = 1.0f / (emitter->per_second * count_multiplier);
    while (emitter->emitting_timer >= emit_delay){
        emitter->emitting_timer -= emit_delay;
        shoot_particle(game, *emitter, direction, start_position);
    }
}


void shoot_particle(Game *game, particle_emitter emitter, Vector2 direction, Vector2 start_position){
    rnd_state = (rnd_state + 1) * 3;
    Particle particle = {};
    particle.entity = {};
    particle.entity.position = start_position;
    
    particle.shape = emitter.shape;
    
    f32 scale = rnd(emitter.scale_min, emitter.scale_max);
    particle.entity.scale = {scale, scale};
    particle.original_scale = {scale, scale};
    
    rnd_state = (rnd_state + 1) * 3;
    f32 x_direction = rnd(direction.x - emitter.spread, direction.x + emitter.spread);
    rnd_state = (rnd_state + 1) * 3;
    f32 y_direction = rnd(direction.y - 0.2f, direction.y + 0.2f);
    Vector2 randomized_direction = {x_direction, y_direction};
                                    
    rnd_state = (rnd_state + 1) * 3;
    f32 randomized_speed = rnd(emitter.speed_min, emitter.speed_max);
    
    rnd_state = (rnd_state + 1) * 3;
    f32 lifetime = rnd(emitter.lifetime_min, emitter.lifetime_max);
    particle.max_lifetime = lifetime;
    
    if (emitter.try_splash){
        if (rnd(0.0f, 1.0f) <= emitter.splash_chance){
            particle.leave_splash = 1;
        } 
    }
    
    if (emitter.colliding_chance >= 1.0f){ 
        particle.colliding = 1;
    } else if (emitter.colliding_chance <= 0){
        particle.colliding = 0;
    } else{
        if (rnd(0.0f, 1.0f) <= emitter.colliding_chance){
            particle.colliding = 1;
        } 
    }
    
    particle.color = emitter.color;
    
    particle.velocity = multiply(randomized_direction, randomized_speed);
    
    array_add(&game->particles, &particle);
}

void shake_camera(Game *game, f32 stress){
    game->shake_trauma = clamp01(game->shake_trauma + stress);
}

void update_camera_shake(Game *game){
    f32 current_shake = game->shake_trauma * game->shake_trauma;
    
    if (current_shake <= 0){
        game->shake_additional_position = {0, 0};
        return;
    }
    
    Vector2 previous_shake_position = game->shake_last_additional_position;
    
    game->shake_last_additional_position = {
                                 game->shake_max_power.x * ((f32)((rnd(100  * (i32)(game->time * 10))) % 10000)) / 10000,
                                 game->shake_max_power.y * ((f32)((rnd(1000 * (i32)(game->time * 10))) % 10000)) / 10000};
    multiply(&game->shake_last_additional_position, current_shake);
    
    add(&game->shake_additional_position, subtract(game->shake_last_additional_position, previous_shake_position));
    
    game->shake_trauma = clamp01(game->shake_trauma - game->delta * game->shake_trauma_decrease);
}

void debug_update(Game *game){
    if (game->input.g_key){
        game->player.entity.position = game->input.mouse_world_position;
    }
}
