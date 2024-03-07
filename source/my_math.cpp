#pragma once

#include <math.h>

#define PI 3.14159265359f

global_variable Vector2 Vector2_one = {1, 1};
global_variable Vector2 Vector2_zero = {0, 0};

f32 normalize(f32 number){
    if (number == 0) return 0;
    if (number > 0) return 1;
    return -1;
}

f32 abs(f32 number){
    if (number < 0) return -number;
    return number;
}

i32 clamp(i32 value, i32 min, i32 max){
    if (value <= min){
        return min;
    }
    
    if (value >= max){
        return max;
    }
    
    return value;
}

f32 clamp(f32 value, f32 min, f32 max){
    if (value <= min){
        return min;
    }
    
    if (value >= max){
        return max;
    }
    
    return value;
}

void clamp(f32 *value, f32 min, f32 max){
    if (*value <= min){
        *value = min;
    } else if (*value >= max){
        *value = max;
    }
}

Vector2 add(Vector2 first, Vector2 second){
    return {first.x + second.x, first.y + second.y};
}

void add(Vector2 *first, Vector2 second){
    first->x += second.x;
    first->y += second.y;
}

Vector2 subtract(Vector2 first, Vector2 second){
    return {first.x - second.x, first.y - second.y};
}

void subtract(Vector2 *first, Vector2 second){
    first->x -= second.x;
    first->y -= second.y;
}

Vector2 multiply(Vector2 vector, f32 value){
    return {vector.x * value, vector.y * value};
}

void multiply(Vector2 *vector, f32 value){
    vector->x *= value;
    vector->y *= value;
}

Vector2 multiply(Vector2 first, Vector2 second){
    return {first.x * second.x, first.y * second.y};
}

Vector2 divide(Vector2 vector, f32 value){
    return {vector.x / value, vector.y / value};
}

f32 sqr_magnitude(Vector2 vector){
    return vector.x * vector.x + vector.y * vector.y;
}

f32 magnitude(Vector2 vector){
    return sqrt(sqr_magnitude(vector));
}

Vector2 normalized(Vector2 vector){
    return divide(vector, magnitude(vector));
}

void normalize(Vector2 *vector){
    *vector = divide(*vector, magnitude(*vector));
}

float lerp(float a, float b, float t){
    return ((float)1-t) * a + b * t;
}

int lerp(int a, int b, float t){
    return ((float)1-t) * a + b * t;
}

Vector2 lerp(Vector2 a, Vector2 b, float t){
    return {lerp(a.x, b.x, t), lerp(a.y, b.y, t)};
}

int lerp_color(uint32_t color1, uint32_t color2, float fraction)
{
        unsigned char   r1 = (color1 >> 16) & 0xff;
        unsigned char   r2 = (color2 >> 16) & 0xff;
        unsigned char   g1 = (color1 >> 8) & 0xff;
        unsigned char   g2 = (color2 >> 8) & 0xff;
        unsigned char   b1 = color1 & 0xff;
        unsigned char   b2 = color2 & 0xff;

        return (int) ((r2 - r1) * fraction + r1) << 16 |
                (int) ((g2 - g1) * fraction + g1) << 8 |
                (int) ((b2 - b1) * fraction + b1);
}

float powe(float value, int power){
    if (power == 0) return 1;
    
    if (power & 1) return value * pow(value, power >> 1) * pow (value, power >> 1);
    else           return pow(value, power >> 1) * pow(value, power >> 1);
}

int factorial(int value){
    if (value <= 1) return 1;
    return value * factorial(value-1);
}

int bezie(int *numbers, int n, float t){
    int result = 0x00;
    for (int i = 0; i < n - 1; i++){
        result += (factorial(n) / (factorial(n - i) * factorial(i))) // Число сочетаний n по i
                  * pow((float)1 - t, n - i)
                  * pow(t, i) * numbers[i];
    }
    
    result += pow(t, n) * numbers[n - 1];
    
    return result;
}

int bezie_colors(uint32_t *colors, int colors_count, float fraction)
{
    int *reds   = (int *)malloc(colors_count * sizeof(int));    
    int *greens = (int *)malloc(colors_count * sizeof(int));    
    int *blues  = (int *)malloc(colors_count * sizeof(int));    
    
    for (int i = 0; i < colors_count; i++){
        reds  [i] = (colors[i] >> 16) & 0xff;
        greens[i] = (colors[i] >> 8)  & 0xff;
        blues [i] =  colors[i]        & 0xff;
    }
    
    int result =  bezie(reds, colors_count, fraction)   << 16 |
                  bezie(greens, colors_count, fraction) << 8  |
                  bezie(blues, colors_count, fraction);
    free(reds);
    free(greens);
    free(blues);
    return result;
}

int lerp_colors(uint32_t *colors, int colors_count, float fraction){
    //clamp(&fraction, (float)0, (float)0.9999999);
    int index  = lerp(0, colors_count-1, fraction);
    int color1 = colors[index];
    int color2 = colors[index+1];
    
    float t = (fraction - ((float)index/(colors_count-1))) * (colors_count-1);
    
    return lerp_color(color1, color2, t);
}


global_variable u32 rnd_state = 0;

//lehmer
u32 rnd(u32 state)
{
	return (u64)state * 48271 % 0x7fffffff;
}

u32 rnd()
{
    rnd_state = (1 + rnd_state) * 5;
	return (u64)rnd_state * 48271 % 0x7fffffff;
}

int rnd(int min, int max){
    return (rnd() % (max - min)) + min;
}

f32 rnd(f32 min, f32 max){
    return ((f32)rnd() / (f32)(0x7FFFFFFF)) * (max - min) + min;
}

f32 rnd01(){
    return rnd(0.0f, 1.0f);
}


//Ease functions
float EaseInOutQuad(float x){
    return x < 0.5f ? 2.0f * x * x : 1 - (f32)pow(-2 * x + 2, 2) / 2.0f;
}

float EaseOutQuint(float x){
    return 1.0f - pow(1.0f - x, 5);
}

float EaseOutElastic(float x){
    float c4 = (2 * PI) / 3.0f;
    
    return x == 0 ? 0
      : (x == 1
      ? 1
      : pow(2.0f, -10 * x) * sin((x * 10 - 0.75f) * c4) + 1.0f);
}

f32 EaseInCirc(f32 x){
    return 1 - sqrt(1 - pow(x, 2));
}

f32 EaseInQuint(f32 x){
    return x * x * x * x * x;
}
