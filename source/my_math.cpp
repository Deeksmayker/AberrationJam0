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
