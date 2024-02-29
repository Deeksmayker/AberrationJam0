#include "aberration.h"

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
    width  *= UNIT_SIZE;
    height *= UNIT_SIZE;
    u8 *row = (u8 *) Buffer->Memory;
    
    u32 upPosition     = (u32)yPosition - (u32)height * 0.5f;
    u32 bottomPosition = (u32)yPosition + (u32)height * 0.5f;
    u32 leftPosition   = (u32)xPosition - (u32)width * 0.5f;
    u32 rightPosition  = (u32)xPosition + (u32)width * 0.5f;
    
    row += Buffer->Pitch * upPosition;
    
    for (u32 y = upPosition; y <= bottomPosition && y < Buffer->Height && y >= 0; y++){
        u32 *pixel = (u32 *)row;
        pixel += leftPosition;
        for (int x = leftPosition; x <= rightPosition && x < Buffer->Width && x >= 0; x++){
            
            *pixel++ = color;
        }
        row += Buffer->Pitch;
    }
}

void draw_rect(screen_buffer *Buffer, Vector2 position, Vector2 size, u32 color){
    draw_rect(Buffer, position.x, position.y, size.x, size.y, color);
}

Entity *entities;
Entity player;

void InitGame(){
    entities = (Entity *)malloc(3 * sizeof(Entity));
    
    player = entities[0];
    player.position = {100, 100};
    player.scale = {2, 2};
}

void GameUpdateAndRender(f32 delta, Input input, screen_buffer *Buffer){
    local_persist int aboba = 100;
    
    RenderFunnyGradient(Buffer, aboba, aboba);
    
    update(delta, input);
    render(Buffer);
    //draw_rect(Buffer, Buffer->Width * 0.5f, Buffer->Height * 0.5f, 100, 100, 0x44332255);

    aboba++;
}

void update(f32 delta, Input input){
    f32 move_speed = 100;
    
    if (input.up_key){
        player.position.y -= move_speed * delta;   
    }
    
    if (input.down_key){
        player.position.y += move_speed * delta;   
    }
}

Vector2 vec2_multiply(Vector2 vector, float value){
    return {vector.x * value, vector.y * value};
}

void render(screen_buffer *Buffer){
    draw_rect(Buffer, player.position, player.scale, 0xff3423);
    //draw_rect(Buffer, player.position.x, player.position.y, player.scale.x * UNIT_SIZE, player.scale.y * UNIT_SIZE, 0xff5533);
}
