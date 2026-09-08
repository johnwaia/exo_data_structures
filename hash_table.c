#include <stdio.h>
#include <stdlib.h>
#include "hash_table.h"

int hash_good(int value) {
    /* value peut etre negatif : on ramene toujours dans [0, TABLE_SIZE). */
    long v = (long)value % TABLE_SIZE;
    if (v < 0) {
        v += TABLE_SIZE;
    }
    return (int)v;
}

int hash_bad(int value) {
    (void)value;
    return 0;
}

void ht_init(HashTable *table) {
    int i;
    for (i = 0; i < TABLE_SIZE; i++) {
        table->buckets[i] = NULL;
    }
}

/* Chainage : O(1) en moyenne avec une bonne fonction, O(n) avec hash_bad. */
void hash_insert_fn(HashTable *table, int value, HashFunc fn) {
    int index = fn(value);
    Entry *entry = (Entry *)malloc(sizeof(Entry));
    if (entry == NULL) {
        fprintf(stderr, "HashTable: echec de malloc\n");
        exit(EXIT_FAILURE);
    }
    entry->value = value;
    entry->next = table->buckets[index];
    table->buckets[index] = entry;
}

int hash_contains_fn(HashTable *table, int value, HashFunc fn) {
    int index = fn(value);
    Entry *current = table->buckets[index];
    while (current != NULL) {
        if (current->value == value) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

void hash_insert(HashTable *table, int value) {
    hash_insert_fn(table, value, hash_good);
}

int hash_contains(HashTable *table, int value) {
    return hash_contains_fn(table, value, hash_good);
}

void hash_free_collection(HashTable *table) {
    int i;
    for (i = 0; i < TABLE_SIZE; i++) {
        Entry *current = table->buckets[i];
        while (current != NULL) {
            Entry *next = current->next;
            free(current);
            current = next;
        }
        table->buckets[i] = NULL;
    }
}
