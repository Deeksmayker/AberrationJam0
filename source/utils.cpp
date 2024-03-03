#pragma once

struct Array{  
    u8 *data;
    size_t size;
    int count;
    int max_count;
};

Array array_init(size_t size, int count = 10){
    Array array = {};
    array.size = size;
    array.data = (u8 *)malloc(count * size);
    array.max_count = count;
    
    return array;
}

u8 *array_get(Array *array, int index){
    return (array->data) + index * array->size;
}

void array_add(Array *array, void *value){
    if (array->count >= array->max_count-1) return;
    u8* element = array_get(array, array->count);
    memmove(element, value, array->size);
    array->count++;
}



