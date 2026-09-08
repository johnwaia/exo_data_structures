#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#define TABLE_SIZE 10007

typedef struct Entry {
    int value;
    struct Entry *next;
} Entry;

typedef struct {
    Entry *buckets[TABLE_SIZE];
} HashTable;

typedef int (*HashFunc)(int);

/* Bonne fonction de hachage : distribution uniforme sur TABLE_SIZE buckets. */
int hash_good(int value);

/* Mauvaise fonction de hachage : tout tombe dans le bucket 0 (pire cas). */
int hash_bad(int value);

void ht_init(HashTable *table);

/* API demandee par l'enonce : utilise hash_good par defaut. */
void hash_insert(HashTable *table, int value);
int  hash_contains(HashTable *table, int value); /* 1 si trouve, 0 sinon */

/* Variantes parametrees, utilisees pour l'experience good vs bad. */
void hash_insert_fn(HashTable *table, int value, HashFunc fn);
int  hash_contains_fn(HashTable *table, int value, HashFunc fn);

void hash_free_collection(HashTable *table);

#endif /* HASH_TABLE_H */
