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

void array_free(Array *array){
    free(array->data);
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

void array_remove(Array *array, int index){
    for (int i = index; i < array->count - 1; i++){
        u8 *current_element = array_get(array, i);
        u8 *next_element = array_get(array, i + 1);
        memmove(current_element, next_element, array->size);
    }
    
    array->count--;
}
