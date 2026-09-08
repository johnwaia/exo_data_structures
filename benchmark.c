#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "dynamic_array.h"
#include "linked_list.h"
#include "hash_table.h"

static const int SIZES[] = {1000, 10000, 100000, 1000000};
#define NUM_SIZES (int)(sizeof(SIZES) / sizeof(SIZES[0]))

/* Repetitions choisies pour que chaque mesure reste au-dessus de la
 * resolution de l'horloge tout en gardant un temps total d'execution
 * raisonnable, meme pour les operations en O(n). */
#define REP_GET        20000  /* O(1) : beaucoup de repetitions pour lisser le bruit */
#define REP_FIND         100  /* O(n) sur array/liste : n * REP_FIND comparaisons  */
#define REP_INSERT_RM    200  /* O(n) sur array (decalage) / liste sans queue      */
#define REP_TRAVERSAL     10  /* O(n) : quelques passages suffisent a moyenner     */
#define REP_HASH_GOOD  20000  /* O(1) en moyenne                                    */
#define REP_HASH_BAD      10  /* O(n) avec hash_bad : tres couteux, peu de repets   */

#define ABSENT_VALUE (-1)

static int *generate_values(int n, unsigned int seed) {
    int *values = (int *)malloc((size_t)n * sizeof(int));
    int i;
    if (values == NULL) {
        fprintf(stderr, "generate_values: malloc echoue\n");
        exit(EXIT_FAILURE);
    }
    srand(seed);
    for (i = 0; i < n; i++) {
        unsigned int raw = ((unsigned int)rand() << 16) ^ (unsigned int)rand();
        int v = (int)(raw & 0x7FFFFFFFu); /* toujours >= 0, jamais ABSENT_VALUE */
        values[i] = v;
    }
    return values;
}

static double time_ms(struct timespec start, struct timespec end) {
    return elapsed_ms(start, end);
}

/* Cherche la plus longue chaine de collisions dans une table (diagnostic). */
static int hash_max_chain(const HashTable *table) {
    int i, max_len = 0;
    for (i = 0; i < TABLE_SIZE; i++) {
        int len = 0;
        Entry *e = table->buckets[i];
        while (e != NULL) {
            len++;
            e = e->next;
        }
        if (len > max_len) {
            max_len = len;
        }
    }
    return max_len;
}

static void bench_dynamic_array(int n, const int *values) {
    DynamicArray arr;
    struct timespec t0, t1;
    long long sum;
    int i, idx, found;

    da_init(&arr);

    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < n; i++) {
        da_insert_back(&arr, values[i]);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("  build (insert_back x n)      : %10.3f ms\n", time_ms(t0, t1));

    idx = n / 2;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < REP_GET; i++) {
        found = da_get(&arr, idx);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    (void)found;
    printf("  get(n/2)                     : %10.6f ms/appel\n", time_ms(t0, t1) / REP_GET);

    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < REP_FIND; i++) {
        found = da_find(&arr, ABSENT_VALUE);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    (void)found;
    printf("  find(valeur absente)         : %10.6f ms/appel\n", time_ms(t0, t1) / REP_FIND);

    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < REP_INSERT_RM; i++) {
        da_insert_front(&arr, -2);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("  insert_front                 : %10.6f ms/appel\n", time_ms(t0, t1) / REP_INSERT_RM);

    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < REP_INSERT_RM; i++) {
        da_insert_back(&arr, -3);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("  insert_back                  : %10.6f ms/appel\n", time_ms(t0, t1) / REP_INSERT_RM);

    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < REP_INSERT_RM; i++) {
        da_remove_front(&arr);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("  remove_front                 : %10.6f ms/appel\n", time_ms(t0, t1) / REP_INSERT_RM);

    sum = 0;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < REP_TRAVERSAL; i++) {
        int j;
        long long local_sum = 0;
        for (j = 0; j < arr.size; j++) {
            local_sum += arr.data[j];
        }
        sum += local_sum;
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("  parcours complet (somme)     : %10.3f ms  (sum controle=%lld)\n",
           time_ms(t0, t1) / REP_TRAVERSAL, sum);

    da_free_collection(&arr);
}

static void bench_linked_list(int n, const int *values) {
    LinkedList list;
    struct timespec t0, t1;
    long long sum;
    int i, idx, found;

    ll_init(&list);

    /* Construction via insert_front : O(1) par insertion. La structure
     * imposee par l'enonce n'ayant pas de pointeur de queue, construire
     * via insert_back couterait O(n^2) et serait irrealisable pour
     * n = 1 000 000. L'insert_back est mesure isolement plus bas. */
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < n; i++) {
        ll_insert_front(&list, values[i]);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("  build (insert_front x n)     : %10.3f ms\n", time_ms(t0, t1));

    idx = n / 2;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < REP_GET; i++) {
        found = ll_get(&list, idx);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    (void)found;
    printf("  get(n/2)                     : %10.6f ms/appel\n", time_ms(t0, t1) / REP_GET);

    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < REP_FIND; i++) {
        found = ll_find(&list, ABSENT_VALUE);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    (void)found;
    printf("  find(valeur absente)         : %10.6f ms/appel\n", time_ms(t0, t1) / REP_FIND);

    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < REP_INSERT_RM; i++) {
        ll_insert_front(&list, -2);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("  insert_front                 : %10.6f ms/appel\n", time_ms(t0, t1) / REP_INSERT_RM);

    /* insert_back : O(n) par appel (parcours jusqu'a la queue). On limite
     * le nombre de repetitions pour rester dans un temps raisonnable. */
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < REP_INSERT_RM; i++) {
        ll_insert_back(&list, -3);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("  insert_back                  : %10.6f ms/appel\n", time_ms(t0, t1) / REP_INSERT_RM);

    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < REP_INSERT_RM; i++) {
        ll_remove_front(&list);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("  remove_front                 : %10.6f ms/appel\n", time_ms(t0, t1) / REP_INSERT_RM);

    sum = 0;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < REP_TRAVERSAL; i++) {
        Node *cur = list.head;
        long long local_sum = 0;
        while (cur != NULL) {
            local_sum += cur->value;
            cur = cur->next;
        }
        sum += local_sum;
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("  parcours complet (somme)     : %10.3f ms  (sum controle=%lld)\n",
           time_ms(t0, t1) / REP_TRAVERSAL, sum);

    ll_free_collection(&list);
}

static void bench_hash_table(int n, const int *values) {
    HashTable good_table, bad_table;
    struct timespec t0, t1;
    int i, found;

    ht_init(&good_table);
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < n; i++) {
        hash_insert_fn(&good_table, values[i], hash_good);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("  [hash_good] build             : %10.3f ms  (chaine max=%d)\n",
           time_ms(t0, t1), hash_max_chain(&good_table));

    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < REP_HASH_GOOD; i++) {
        found = hash_contains_fn(&good_table, ABSENT_VALUE, hash_good);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    (void)found;
    printf("  [hash_good] contains(absent)  : %10.6f ms/appel\n", time_ms(t0, t1) / REP_HASH_GOOD);

    ht_init(&bad_table);
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < n; i++) {
        hash_insert_fn(&bad_table, values[i], hash_bad);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("  [hash_bad]  build              : %10.3f ms  (chaine max=%d)\n",
           time_ms(t0, t1), hash_max_chain(&bad_table));

    /* O(n) par appel : tout est dans le bucket 0, peu de repetitions. */
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < REP_HASH_BAD; i++) {
        found = hash_contains_fn(&bad_table, ABSENT_VALUE, hash_bad);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    (void)found;
    printf("  [hash_bad]  contains(absent)  : %10.6f ms/appel\n", time_ms(t0, t1) / REP_HASH_BAD);

    hash_free_collection(&good_table);
    hash_free_collection(&bad_table);
}

int main(void) {
    int s;
    for (s = 0; s < NUM_SIZES; s++) {
        int n = SIZES[s];
        int *values = generate_values(n, 42u);

        printf("================================================================\n");
        printf("n = %d\n", n);

        printf("-- Dynamic Array ------------------------------------------------\n");
        bench_dynamic_array(n, values);

        printf("-- Linked List --------------------------------------------------\n");
        bench_linked_list(n, values);

        printf("-- Hash Table (good vs bad) --------------------------------------\n");
        bench_hash_table(n, values);

        free(values);
    }
    printf("================================================================\n");
    return 0;
}
