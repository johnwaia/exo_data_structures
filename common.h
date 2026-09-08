#ifndef COMMON_H
#define COMMON_H

/*
 * L'enonce impose une horloge monotone POSIX (clock_gettime / CLOCK_MONOTONIC).
 * Sous Linux/gcc, time.h fournit tout cela nativement : rien n'est modifie.
 * Sous MSVC (pas de clock_gettime), on fournit une implementation equivalente
 * basee sur QueryPerformanceCounter, avec la meme signature, pour pouvoir
 * developper/tester ce projet sur Windows sans changer une ligne du code
 * de mesure.
 */

#include <time.h>

#if defined(_MSC_VER)

#include <windows.h>

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif

/* UCRT (VS2015+) definit deja struct timespec dans <time.h>, on la reutilise
 * telle quelle et on se contente d'ajouter clock_gettime/CLOCK_MONOTONIC. */

static __inline int clock_gettime(int clk_id, struct timespec *ts) {
    static LARGE_INTEGER freq;
    static int have_freq = 0;
    LARGE_INTEGER counter;
    (void)clk_id;

    if (!have_freq) {
        QueryPerformanceFrequency(&freq);
        have_freq = 1;
    }
    QueryPerformanceCounter(&counter);

    ts->tv_sec = (long long)(counter.QuadPart / freq.QuadPart);
    ts->tv_nsec = (long)(((counter.QuadPart % freq.QuadPart) * 1000000000LL) / freq.QuadPart);
    return 0;
}

#endif /* _MSC_VER */

/* Renvoie la duree entre deux timespec, en millisecondes (double). */
static __inline double elapsed_ms(struct timespec start, struct timespec end) {
    double sec = (double)(end.tv_sec - start.tv_sec);
    double nsec = (double)(end.tv_nsec - start.tv_nsec);
    return sec * 1000.0 + nsec / 1000000.0;
}

#endif /* COMMON_H */
