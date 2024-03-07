global_variable u32 rand_seed = 0;

#include "utils.cpp"
#include "aberration.h"
#include "my_math.cpp"

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

Game global_game;

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


void draw_rect(screen_buffer *Buffer, f32 xPosition, f32 yPosition, f32 width, f32 height, u32 color){
    f32 screen_world_height = (f32)Buffer->Height / unit_size;
    if (global_game.top_left_world_position.y - camera_position.y < screen_world_height){
        
    }

    xPosition -= camera_position.x;
    yPosition -= camera_position.y;
    
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
            if (x >= Buffer->Width && x < 0){
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



void draw_line(screen_buffer *Buffer, f32 start_x, f32 start_y, f32 end_x, f32 end_y, f32 width, u32 color){
    Vector2 vector_to_end = subtract({end_x, end_y}, {start_x, start_y});
    int points_count = (((int)magnitude(vector_to_end) + 10) * 4);
    
    for (int i = 0; i <= points_count; i++){
        f32 t = lerp((f32)0, (f32)1, (f32)i / (f32)points_count);
        
        f32 x = lerp(start_x, end_x, t);
        f32 y = lerp(start_y, end_y, t);
        
        draw_rect(Buffer, x, y, width, width, color);
    }
}
void draw_line(screen_buffer *Buffer, Vector2 start_position, Vector2 end_position, f32 width, u32 color){
    draw_line(Buffer, start_position.x, start_position.y, end_position.x, end_position.y, width, color);
}


void fill_background(screen_buffer *Buffer, u32 color){
    u8 *ROW = (u8 *) Buffer->Memory;
    for (int y = 0; y < Buffer->Height; y++){
        u32 *Pixel = (u32 *) ROW;
        for (int x = 0; x < Buffer->Width; x++){
            *Pixel++ = color;                  
        }
        ROW += Buffer->Pitch;
    }
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
    
    add_fly_enemy(&global_game, {30, 35});
    
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
    
    particle_emitter *shoot_emitter = &global_game.player.shoot.wall_hit_emitter;
    *shoot_emitter = {};
    shoot_emitter->speed_min    = 40;
    shoot_emitter->speed_max    = 150;
    shoot_emitter->scale_min    = 0.4f;
    shoot_emitter->scale_max    = 1.5f;
    shoot_emitter->count_min    = 30;
    shoot_emitter->count_max    = 80;
    shoot_emitter->lifetime_min = 0.1f;
    shoot_emitter->lifetime_max = 0.5f;
    shoot_emitter->shape = (particle_shape)0;
    
    global_game.top_left_world_position     = {0, (f32)global_game.tilemap.rows * global_game.tilemap.block_scale};
    global_game.bottom_right_world_position = {(f32)global_game.tilemap.columns * global_game.tilemap.block_scale, 0};
    
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

    
    
    //fill_level1_tilemap(&global_game);
}


global_variable f32 previous_delta = 0;

void GameUpdateAndRender(f32 delta, Input input, screen_buffer *Buffer){
    rand_seed = (u32)(global_game.delta * 10000000000.0f);
    rnd_state = rand_seed;
    
    //unit_size = lerp(unit_size, 0.1f, delta * 0.5f);

    f32 mouse_world_position_x = ((f32)input.mouse_screen_position.x /
                                 ((f32)Buffer->ScreenWidth / (f32)Buffer->Width)) / 
                                 unit_size;
    f32 mouse_world_position_y = ((f32)input.mouse_screen_position.y /
                                 ((f32)Buffer->ScreenHeight / (f32)Buffer->Height)) / 
                                 unit_size;
                                 
    input.mouse_world_position = {mouse_world_position_x, mouse_world_position_y + camera_position.y};
    
    f32 screen_world_height = (f32)Buffer->Height / unit_size;
    if(global_game.top_left_world_position.y - camera_position.y < screen_world_height
                            && global_game.player.entity.position.y < screen_world_height){
        global_game.camera_player_position.y = global_game.player.entity.position.y + global_game.top_left_world_position.y; 
        global_game.camera_player_position.x = global_game.player.entity.position.x;
    } else{
        global_game.camera_player_position = global_game.player.entity.position;
    }


    global_game.input = input;
    global_game.time += delta;
    
    f32 frame_time = 0.0166666f;
    
    delta += previous_delta;
    previous_delta = 0;
    
    if (delta > frame_time){
        global_game.delta = frame_time;
        while(delta > frame_time){
            update(&global_game);
            delta -= frame_time;
        }
        
        previous_delta = delta;
    } 
    
    global_game.delta = delta;
    update(&global_game);
    
    
    f32 camera_target_y = global_game.camera_player_position.y - ((f32)Buffer->Height / (f32)unit_size) * 0.5f;
    camera_position.y = lerp(camera_position.y, camera_target_y, global_game.delta * 5);
    loop_world_position(&global_game, &camera_position);

    
    render(&global_game, Buffer);
}

void update(Game *game){
    update_player(game);
    update_particles(game);
    debug_update(game);

    game->player.entity.position.x += game->player.velocity.x * game->delta;
    game->player.entity.position.y += game->player.velocity.y * game->delta;
    
    loop_world_position(game, &game->player.entity.position);
    
    //camera_position = game->player.entity.position;
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
    }
    
    if (game->player.melee_cooldown_timer > 0){
        game->player.melee_cooldown_timer -= game->delta;
    }
    
    if (game->input.mouse_right_key && player->melee_cooldown_timer <= 0){
        player->melee_cooldown_timer = 0.5f;
        Vector2 direction = subtract(game->input.mouse_world_position, player->entity.position);
        emit_particles(game, player->shoot.wall_hit_emitter, direction, player->entity.position, 1.0f);
    }
    
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
    if (game->input.mouse_left_key){
        shoot->holding = 1;
        shoot->holding_timer += game->delta;
        
        if (shoot->holding_timer >= shoot->perfect_hold_time && !shoot->perfect_charged){
            shoot->perfect_charged = 1;
            
            emit_particles(game, shoot->wall_hit_emitter, player_to_mouse, add(player->entity.position, player_to_mouse), 1.0f);
            
        }
        //shooting
    } else if (shoot->holding && player->shoot.cooldown_timer <= 0){
        shoot->holding = 0;
        shoot->perfect_charged = 0;
        
        b32 perfect_shoot = shoot->holding_timer >= shoot->perfect_hold_time - 0.05f
                         && shoot->holding_timer <= (shoot->perfect_hold_time + shoot->perfect_buffer);
                         
        shoot->just_shoot = perfect_shoot ? 2 : 1;
        player->shoot.cooldown_timer = perfect_shoot ? 0.1f : player->shoot.cooldown;
        
        shoot->holding_timer = 0;
    
        f32 shoot_length = perfect_shoot ? 600 : 40;
        
        //recoil
        add(&player->velocity, multiply(multiply(player_to_mouse, -1), perfect_shoot ? shoot-> push_force : shoot->push_force * 0.3f));
        
        Vector2 end_point = add(player->entity.position, multiply(player_to_mouse, shoot_length));
        Line line = {};
        line.start_position = player->entity.position;
        line.end_position   = end_point;
        line.width = perfect_shoot ? 1 : 0.5f;
        
        //shoot line render
        line_entity visual_line = {};
        visual_line.line = line;
        visual_line.line.width = perfect_shoot ? 2.0f : 1.0f;
        visual_line.color = perfect_shoot ? 0xbf212f : 0xf9a73e;
        visual_line.max_lifetime = perfect_shoot ? 0.4f : 0.2f;
        
        array_add(&game->line_entities, &visual_line);
        
        //@TODO: Free this hits
        line_hits hits = check_line_collision(game, line);
        int collision_count = hits.enter_count >= hits.exit_count ? hits.enter_count : hits.exit_count;
        
        for (int i = 0; i < collision_count; i++){
            if (hits.enter_count > i){
                emit_particles(game, player->shoot.wall_hit_emitter, multiply(player_to_mouse, -1), hits.enter_positions[i], 0.5f);
            }
            if (hits.exit_count > i){
                emit_particles(game, player->shoot.wall_hit_emitter, player_to_mouse, hits.exit_positions[i], 1.0f);
            }
            
            if (hits.enter_count > i && hits.exit_count > i){
                //bullet line in tiles
                Line wall_line = {};
                wall_line.start_position = hits.enter_positions[i];
                wall_line.end_position   = hits.exit_positions[i];
                wall_line.width = 0.01f;
                
                line_entity wall_visual_line = {};
                wall_visual_line.line = wall_line;
                wall_visual_line.max_lifetime = 0;
                
                if (hits.hit_types[i] == 1){
                    wall_visual_line.color = 0xffffff;
                    array_add(&game->line_entities, &wall_visual_line);
                } else if (hits.hit_types[i] == 2){
                    wall_visual_line.color = 0xcc3333;
                    subtract(&wall_visual_line.line.start_position, hits.enemy_hits[i]->entity.position);
                    subtract(&wall_visual_line.line.end_position, hits.enemy_hits[i]->entity.position);
                    array_add(&hits.enemy_hits[i]->lines, &wall_visual_line);
                }
            }
        }
    } else{
        shoot->holding_timer = 0;
    }
    
    clamp(&player->velocity.y, -150, 200);
    
    check_player_collisions(game);
    //printf("%d\n", (int)game->player.velocity.x);
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
    fly_enemy enemy = {};
    enemy.entity.position = {30, 35};
    enemy.entity.scale = {3, 4};
    
    enemy.lines = array_init(sizeof(line_entity), 100);
    
    array_add(&global_game.fly_enemies, &enemy);
}


void check_player_collisions(Game *game){
    game->player.grounded = 0;
    
    calculate_player_tilemap_collisions(game, check_tilemap_collisions(game, game->player.velocity, game->player.entity));
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
                add(&player->velocity, multiply(collisions_data[i].normal, magnitude(multiply(player->velocity, collisions_data[i].normal))));
            } break;
            
            case Pole:{
                if (game->input.up_key){
                    player->velocity.y += 100 * game->delta;
                    player->velocity.x *= 1.0f - game->delta * 1;
                    player->riding_pole = 1;
                    
                    update_overtime_emitter(game, &player->pole_ride_emitter, {0, -1}, collisions_data[i].obstacle_position);
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
        
        calculate_particle_tilemap_collisions(game, particle, check_tilemap_collisions(game, particle->velocity, particle->entity));
        
        Vector2 next_position = add(particle->entity.position, multiply(particle->velocity, game->delta));
        
        particle->entity.position = next_position;
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
    int points_count = (((int)magnitude(vector_to_end) + 1) * 4);
    
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
        line_point_entity.scale = {line.width, line.width};
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
    fill_background(Buffer, 0xffffff);
    
    draw_entities(game, Buffer);

    Player *player = &game->player;
    draw_rect(Buffer, player->entity.position, player->entity.scale, 0xff3423);
    //Vector2 added = add(game->player.entity.position, {10, 10});
    
    Vector2 player_to_mouse = subtract(game->input.mouse_world_position, game->camera_player_position);
    normalize(&player_to_mouse);
    
    //Draw Rifle
    local_persist f32 length_multiplier = 1.0f; 
    Player::shooter *shoot = &player->shoot;
    if (shoot->holding){
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
              1.7f, 0x555555);
    draw_line(Buffer,
              add(player->entity.position, multiply(player_to_mouse, 1.5f * length_multiplier)),
              add(player->entity.position, multiply(player_to_mouse, 2.5f * middle_length_multiplier)),
              1.3f, 0x777777);
    draw_line(Buffer,
              add(player->entity.position, multiply(player_to_mouse, 3.0f * length_multiplier)),
              add(player->entity.position, multiply(player_to_mouse, 4.5f * end_length_multiplier)),
              0.7f, 0x999999);
    
    //draw_line(Buffer, game->player.entity.position, game->input.mouse_world_position, 0.2f, 0xff5533);
    
    //draw_rect(Buffer, game->input.mouse_world_position.x, game->input.mouse_world_position.y, 3, 3, 0xbc32fd);
    //draw_rect(Buffer, player.entity.position.x, player.entity.position.y, player.scale.x * unit_size, player.scale.y * unit_size, 0xff5533);
}

void draw_entities(Game *game, screen_buffer *Buffer){
    for (int y = 0; y < game->tilemap.rows; y++){
        for (int x = 0; x < game->tilemap.columns; x++){
            i32 position_y = game->tilemap.rows - y - 1;
            i32 position_x = x;

            switch ((TileType)level1[y][x]){
                case None:{
                    continue;
                } break;
                case Ground:{
                    draw_rect(Buffer, position_x * game->tilemap.block_scale, position_y * game->tilemap.block_scale,
                              game->tilemap.block_scale, game->tilemap.block_scale, 0x66ffff);
                } break;
                case Pole:{
                    draw_rect(Buffer, position_x * game->tilemap.block_scale, position_y * game->tilemap.block_scale,
                              game->tilemap.block_scale * 0.2, game->tilemap.block_scale, 0x22ff22);
                } break;
            }
            
        }
    }
    
    //particles
    for (int i = 0; i < game->particles.count; i++){
        Particle *particle = ((Particle *)array_get(&game->particles, i));
        
        if (particle->shape == (particle_shape)0){
            draw_rect(Buffer, particle->entity.position, particle->entity.scale, particle->color);
        } else if (particle->shape == (particle_shape)1){
            Vector2 end_position = add(particle->entity.position, multiply(particle->velocity, 0.1f));
            draw_line(Buffer, particle->entity.position, end_position, particle->entity.scale.x, particle->color);
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
                line->visual_width = lerp(0.0f, line->line.width, EaseOutQuint(t));
            } else{
                f32 t = (line->lifetime - time_to_max) / (line->max_lifetime - time_to_max);
                line->visual_width = lerp(line->line.width, 0.0f, EaseInCirc(t));
            }
        } else{
            line->visual_width = line->line.width;
        }
        
        draw_line(Buffer, line->line.start_position, line->line.end_position, line->visual_width, line->color);
    }
    
    //draw enemies
    for  (int i = 0; i < game->fly_enemies.count; i++){ 
        fly_enemy *enemy = (fly_enemy *)array_get(&game->fly_enemies, i);
        
        enemy->entity.position.x += game->delta;
        
        draw_rect(Buffer, enemy->entity.position, enemy->entity.scale, 0x3366aa);
        
        for (int j = 0; j < enemy->lines.count; j++){
            line_entity *bullet_hole = (line_entity *)array_get(&enemy->lines, j);
            Vector2 start_position = add(bullet_hole->line.start_position, enemy->entity.position);
            Vector2 end_position = add(bullet_hole->line.end_position, enemy->entity.position);
            draw_line(Buffer, start_position, end_position, bullet_hole->visual_width, bullet_hole->color);
        }
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

void update_overtime_emitter(Game *game, particle_emitter *emitter, Vector2 direction, Vector2 start_position){
    emitter->emitting_timer += game->delta;
    f32 emit_delay = 1.0f / emitter->per_second;
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
    
    particle.color = emitter.color;
    
    particle.velocity = multiply(randomized_direction, randomized_speed);
    
    array_add(&game->particles, &particle);
    
}

void debug_update(Game *game){
    if (game->input.g_key){
        game->player.entity.position = game->input.mouse_world_position;
    }
}
