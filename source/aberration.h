#pragma once

#if DEBUG
#define assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define assert(Expression)
#endif

#define UNIT_SIZE 50.0f;

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
};

struct screen_buffer{
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel = 4;
};


void draw_rect(screen_buffer *Buffer, f32 xPosition, f32 yPosition, f32 width, f32 height, u32 color);
void draw_rect(screen_buffer *Buffer, Vector2 position, Vector2 size, u32 color);

// Time, Input, Bitmap buffer, Sound buffer
void GameUpdateAndRender(f32 delta, Input input, screen_buffer *Buffer);
void update(f32 delta, Input input);
void render(screen_buffer *Buffer);
