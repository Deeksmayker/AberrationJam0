f32 normalize(f32 number){
    if (number == 0) return 0;
    if (number > 0) return 1;
    return -1;
}

f32 abs(f32 number){
    if (number < 0) return -number;
    return number;
}
