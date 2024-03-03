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

    width  *= UNIT_SIZE;
    height *= UNIT_SIZE;
    xPosition *= UNIT_SIZE;
    yPosition *= UNIT_SIZE;
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
    
    global_game.player.shoot_emmiter = {};
    global_game.player.shoot_emmiter.speed_min = 20;
    global_game.player.shoot_emmiter.speed_max = 50;
    global_game.player.shoot_emmiter.scale_min = 1;
    global_game.player.shoot_emmiter.scale_max = 3;
    global_game.player.shoot_emmiter.count_min = 10;
    global_game.player.shoot_emmiter.count_max = 30;
    
    rnd_state = 465456465;
    printf("\n%f", rnd(0.0f, 1.0f)); 
    rnd_state = 465456466;
    
    printf("\n%f", rnd(0.0f, 1.0f)); 
    
    //fill_level1_tilemap(&global_game);
}

void GameUpdateAndRender(f32 delta, Input input, screen_buffer *Buffer){
    rand_seed = (u32)(global_game.delta * 100000000000.0f);
    rnd_state = rand_seed;

    f32 mouse_world_position_x = ((f32)input.mouse_screen_position.x /
                                 ((f32)Buffer->ScreenWidth / (f32)Buffer->Width)) / 
                                 global_game.unit_size;
    f32 mouse_world_position_y = ((f32)input.mouse_screen_position.y /
                                 ((f32)Buffer->ScreenHeight / (f32)Buffer->Height)) / 
                                 global_game.unit_size;
                                 
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

    f32 vertical = 0;
    if (game->input.up_key) vertical = 1;
    if (game->input.down_key) vertical = -1;
    
    f32 horizontal = 0;
    if (game->input.right_key) horizontal = 1;
    if (game->input.left_key)  horizontal = -1;
    
    f32 wish_speed = horizontal * game->player.base_speed;
    
    accelerate(&(game->player.velocity), game->delta,
               game->player.base_speed, wish_speed, horizontal,
               game->player.grounded ? game->player.acceleration : game->player.air_acceleration);
    
    if (game->player.grounded){
        apply_friction(&(game->player.velocity), game->player.base_speed, game->delta, game->player.friction);
    }
    
    if (!game->player.grounded){
        game->player.velocity.y += game->gravity * game->delta;
    }
    
    if (game->input.jump_key && game->player.grounded){
        game->player.velocity.y += game->player.jump_force;
        
        game->player.velocity.x += horizontal * game->player.jump_speed_boost;
    }
    
    if (game->player.melee_cooldown_timer > 0){
        game->player.melee_cooldown_timer -= game->delta;
    }
    
    
    if (game->input.mouse_left_key && game->player.melee_cooldown_timer <= 0){
        setup_particles(game, game->player.shoot_emmiter);
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

void update_particles(Game *game){
    for (int i = 0; i < game->particles.count; i++){
        Particle *particle = ((Particle *)array_get(&game->particles, i));
        particle->lifetime += game->delta;
        particle->velocity.y += game->gravity * game->delta;
        
        Vector2 vertical_wish_position = 
               {particle->entity.position.x,
                particle->entity.position.y + particle->velocity.y * game->delta};
        Vector2 horizontal_wish_position = 
               {particle->entity.position.x + particle->velocity.x * game->delta,
                particle->entity.position.y};
        
        if(check_entity_collisions(game, &particle->entity, vertical_wish_position)){
            if (particle->velocity.y <= 0){
                apply_friction(&particle->velocity, 50, game->delta, game->player.friction);
            }
            particle->velocity.y -= particle->velocity.y * 1.5f;
        }
        if(check_entity_collisions(game, &particle->entity, horizontal_wish_position)){
            particle->velocity.x -= particle->velocity.x * 1.5f;
        }
        
        Vector2 next_position = add(particle->entity.position, multiply(particle->velocity, game->delta));
        
        particle->entity.position = next_position;
    }
}

void check_player_collisions(Game *game){
    game->player.grounded = 0;

    Vector2 vertical_wish_position = 
           {game->player.entity.position.x,
            game->player.entity.position.y + game->player.velocity.y * game->delta};
    Vector2 horizontal_wish_position = 
           {game->player.entity.position.x + game->player.velocity.x * game->delta,
            game->player.entity.position.y};
            
    if (check_entity_collisions(game, &game->player.entity, vertical_wish_position)){
        if (game->player.velocity.y < 0){
            game->player.grounded = 1;
        }
        game->player.velocity.y -= game->player.velocity.y;
    }
    
    if (check_entity_collisions(game, &game->player.entity, horizontal_wish_position)){
        game->player.velocity.x -= game->player.velocity.x;
    }
}


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
    
    //Tile collisions
    for (int y = 0; y < game->tilemap.rows; y++){
        for (int x = 0; x < game->tilemap.columns; x++){
            if (level1[y][x] == 0 || level1[y][x] == 2) continue;
            
            Entity tile_entity = {};
            tile_entity.position = {(f32)x * game->tilemap.block_scale, ((f32)game->tilemap.rows - y - 1) * game->tilemap.block_scale}; 
            tile_entity.scale = {(f32)game->tilemap.block_scale, (f32)game->tilemap.block_scale};
            if (check_box_collision(&future_entity, &tile_entity)){
                return 1;
            }
        }
    }

    for (int i = 0; i < game->particles.count; i++){
        Particle *particle = (Particle *)array_get(&game->particles, i);
        if (particle->lifetime < 1) {
            continue;
        }
        if (check_box_collision(&future_entity, &(particle->entity))){
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
    
    //draw_rect(Buffer, game->input.mouse_world_position.x, game->input.mouse_world_position.y, 3, 3, 0xbc32fd);
    //draw_rect(Buffer, player.entity.position.x, player.entity.position.y, player.scale.x * UNIT_SIZE, player.scale.y * UNIT_SIZE, 0xff5533);
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
        draw_rect(Buffer, particle->entity.position, particle->entity.scale, 0x445663);
    }
    /*
    for (int i = 0; i < game->walls.count; i++){
        //printf("%d\n",  (int)((Entity *)array_get(&game->walls, 1))->position.y);
        draw_rect(Buffer, ((Entity *)array_get(&game->walls, i))->position, ((Entity *)array_get(&game->walls, i))->scale, 0x44aaaa);
    }
    */
}

/*
void fill_level1_tilemap(Game *game){
    game->tilemap.tiles = (int **)malloc(game->tilemap.rows * sizeof(int));
    
    for (int i = 0; i < game->tilemap.rows; i++){
        game->tilemap.tiles[i] = (int *)malloc(game->tilemap.columns * sizeof(int));
    }
    
    for (int y = 0; y < game->tilemap.rows; y++){
        for (int x = 0; x < game->tilemap.columns; x++){
            game->tilemap.tiles[y][x] = level1[y][x];
        }
    }
    
    tiles = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

}
*/

void setup_particles(Game *game, particle_emmiter emmiter){
    Vector2 direction = subtract(game->input.mouse_world_position, game->player.entity.position);
    normalize(&direction);
    rnd_state++;
    int count = rnd((int)game->player.shoot_emmiter.count_min, (int)game->player.shoot_emmiter.count_max);
    
    for (int i = 0; i < count; i++){
        rnd_state += i * count * 2344;
        Particle particle = {};
        particle.entity = {};
        particle.entity.position = game->player.entity.position;
        f32 scale = rnd(game->player.shoot_emmiter.scale_min, game->player.shoot_emmiter.scale_max);
        particle.entity.scale = {scale, scale};
        particle.velocity = multiply({rnd(direction.x-0.2f, direction.x + 0.2f), rnd(direction.y-0.2f, direction.y+0.2f)}, rnd(game->player.shoot_emmiter.speed_min, game->player.shoot_emmiter.speed_max));
        
        array_add(&game->particles, &particle);
    }
}

void debug_update(Game *game){
    if (game->input.mouse_right_key){
        game->player.entity.position = game->input.mouse_world_position;
    }
}
