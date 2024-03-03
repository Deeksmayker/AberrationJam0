#pragma once

#include <math.h>

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

Vector2 add(Vector2 first, Vector2 second){
    return {first.x + second.x, first.y + second.y};
}

Vector2 subtract(Vector2 first, Vector2 second){
    return {first.x - second.x, first.y - second.y};
}

Vector2 multiply(Vector2 vector, f32 value){
    return {vector.x * value, vector.y * value};
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

global_variable u32 rnd_state = 0;

//lehmer
u32 rnd(u32 state)
{
	return (u64)state * 48271 % 0x7fffffff;
}

u32 rnd()
{
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


