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

void draw_rect(screen_buffer *Buffer, f32 xPosition, f32 yPosition, f32 width, f32 height, u32 color){
    xPosition -= camera_position.x;
    yPosition -= camera_position.y;

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
    int points_count = (int)magnitude(vector_to_end) * 3;
    
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

Game global_game;

void InitGame(){
    global_game = {};
    global_game.entities = array_init(sizeof(Entity));
    
    global_game.player = {};
    global_game.player.entity = {};
    global_game.player.entity.position = {30, 30};
    global_game.player.entity.scale = {3, 3};
    
    global_game.particles = array_init(sizeof(Particle), 100000);
    
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
    
    particle_emitter *emitter = &global_game.player.shoot_emitter;
    *emitter = {};
    emitter->speed_min    = 40;
    emitter->speed_max    = 150;
    emitter->scale_min    = 0.15f;
    emitter->scale_max    = 0.5f;
    emitter->count_min    = 30;
    emitter->count_max    = 80;
    emitter->lifetime_min = 0.1f;
    emitter->lifetime_max = 0.5f;
    
    //fill_level1_tilemap(&global_game);
}

void GameUpdateAndRender(f32 delta, Input input, screen_buffer *Buffer){
    rand_seed = (u32)(global_game.delta * 100000000000.0f);
    rnd_state = rand_seed;
    
    //unit_size = lerp(unit_size, 0.1f, delta * 0.5f);

    f32 mouse_world_position_x = ((f32)input.mouse_screen_position.x /
                                 ((f32)Buffer->ScreenWidth / (f32)Buffer->Width)) / 
                                 unit_size;
    f32 mouse_world_position_y = ((f32)input.mouse_screen_position.y /
                                 ((f32)Buffer->ScreenHeight / (f32)Buffer->Height)) / 
                                 unit_size;
                                 
    input.mouse_world_position = {mouse_world_position_x, mouse_world_position_y};

    global_game.input = input;
    global_game.delta = delta;
    global_game.time += delta;
    
    update(&global_game);
    render(&global_game, Buffer);
}

void update(Game *game){
    update_player(game);
    update_particles(game);
    debug_update(game);

    game->player.entity.position.x += game->player.velocity.x * game->delta;
    game->player.entity.position.y += game->player.velocity.y * game->delta;
    
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
    
    
    if (game->input.mouse_left_key && game->player.melee_cooldown_timer <= 0){
        Vector2 direction = subtract(game->input.mouse_world_position, game->player.entity.position);
        emit_particles(game, game->player.shoot_emitter, direction, player->entity.position);
        game->player.melee_cooldown_timer = 0.5f;
    }
    
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

void check_player_collisions(Game *game){
    game->player.grounded = 0;
    
    calculate_player_tilemap_collisions(game, check_tilemap_collisions(game, game->player.velocity, game->player.entity));
    

    //calculate_player_tilemap_collision(&game->player, check_tilemap_collisions(game, {0, game->player.velocity.y}, game->player.entity));
    //calculate_player_tilemap_collision(&game->player, check_tilemap_collisions(game, {game->player.velocity.x, 0}, game->player.entity));
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
            tile_entity.position = {(f32)x * game->tilemap.block_scale, ((f32)game->tilemap.rows - y - 1) * game->tilemap.block_scale}; 
            tile_entity.scale = {(f32)game->tilemap.block_scale, (f32)game->tilemap.block_scale};
            
            
            if (check_box_collision(&vertical_future_entity, &tile_entity)){
                collisions_data[collided_count].normal = {0, -normalize(velocity.y)};
                collisions_data[collided_count].tile_type = (TileType)level1[y][x];
                collisions_data[collided_count].collided = 1;
                collided_count++;
            }
            if (velocity.x != 0 && check_box_collision(&horizontal_future_entity, &tile_entity)){
                collisions_data[collided_count].normal = {-normalize(velocity.x), 0};
                collisions_data[collided_count].tile_type = (TileType)level1[y][x];
                collisions_data[collided_count].collided = 1;
                collided_count++;
            }
        }
    }
    
    return collisions_data;
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


//@OBSOLETE
b32 check_entity_collisions(Game *game, Entity *entity, Vector2 wish_position){
    Entity future_entity = *entity;
    future_entity.position = wish_position;
    for (int i = 0; i < game->walls.count; i++){
        //printf("truly wall we check %d\n", (int)((Entity *)array_get(&game->walls, 0))->position.y);
        //printf("truly player we check %d\n", (int)game->player.entity.position.y);
        
        if (check_box_collision(&future_entity, (Entity *)array_get(&game->walls, i))){
            return 1;
        }
    }
    return 0;
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
    if (solution){
        return 1;
    }
    
    return 0;
}



void render(Game *game, screen_buffer *Buffer){
    fill_background(Buffer, 0xffffff);
    
    draw_entities(game, Buffer);

    draw_rect(Buffer, game->player.entity.position, game->player.entity.scale, 0xff3423);
    //Vector2 added = add(game->player.entity.position, {10, 10});
    draw_line(Buffer, game->player.entity.position, game->input.mouse_world_position, 0.2f, 0xff5533);
    
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
        Vector2 end_position = add(particle->entity.position, multiply(particle->velocity, 0.05f));
        draw_line(Buffer, particle->entity.position, end_position, particle->entity.scale.x, 0x445663);
    }
}

void emit_particles(Game *game, particle_emitter emitter, Vector2 direction, Vector2 start_position){
    normalize(&direction);
    rnd_state++;
    int count = rnd((int)emitter.count_min, (int)emitter.count_max);
    
    for (int i = 0; i < count; i++){
        shoot_particle(game, emitter, direction, start_position, i+1);
    }
}

void shoot_particle(Game *game, particle_emitter emitter, Vector2 direction, Vector2 start_position, u32 seed_multiplier){
    rnd_state = (rnd_state + 1) * 3;
    Particle particle = {};
    particle.entity = {};
    particle.entity.position = start_position;
    
    f32 scale = rnd(emitter.scale_min, emitter.scale_max);
    particle.entity.scale = {scale, scale};
    particle.original_scale = {scale, scale};
    
    rnd_state = (rnd_state + 1) * 3;
    f32 x_direction = rnd(direction.x - 0.2f, direction.x + 0.2f);
    rnd_state = (rnd_state + 1) * 3;
    f32 y_direction = rnd(direction.y - 0.2f, direction.y + 0.2f);
    Vector2 randomized_direction = {x_direction, y_direction};
                                    
    rnd_state = (rnd_state + 1) * 3;
    f32 randomized_speed = rnd(emitter.speed_min, emitter.speed_max);
    
    rnd_state = (rnd_state + 1) * 3;
    f32 lifetime = rnd(emitter.lifetime_min, emitter.lifetime_max);
    particle.max_lifetime = lifetime;
    
    particle.velocity = multiply(randomized_direction, randomized_speed);
    
    array_add(&game->particles, &particle);
    
}

void debug_update(Game *game){
    if (game->input.g_key){
        game->player.entity.position = game->input.mouse_world_position;
    }
}
