#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

typedef struct {
    int *data;
    int size;
    int capacity;
} DynamicArray;

void da_init(DynamicArray *arr);
void da_insert_front(DynamicArray *arr, int value);
void da_insert_back(DynamicArray *arr, int value);
int  da_find(const DynamicArray *arr, int value);      /* index ou -1 */
int  da_get(const DynamicArray *arr, int index);
void da_remove_front(DynamicArray *arr);
void da_free_collection(DynamicArray *arr);

#endif /* DYNAMIC_ARRAY_H */
