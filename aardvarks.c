#include "/comp/111/assignments/aardvarks/anthills.h"
#include <semaphore.h>
#include <time.h>

#define TRUE 1
#define FALSE 0

int initialized=FALSE; // semaphores and mutexes are not initialized 
// int cleanup=FALSE; // sempahores and mutexes are not cleaned up

// int array track anthills
int left_ants[ANTHILLS];
// define three semaphores for each antHill
sem_t semaphores[ANTHILLS];
// define three locks
pthread_mutex_t locks[3];
// for nanosleep specify the sleep time
struct timespec t1, t2;

// add lock for critical sections in minus ants in left_ants
static void slurp_with_lock(char aname, int anthill) {
    pthread_mutex_lock(&locks[0]);
    // if current anthill still have ants decrement by 1
    if (left_ants[anthill]-- > 0) {
        pthread_mutex_unlock(&locks[0]);
        if (slurp(aname, anthill) != 1) {
            pthread_mutex_lock(&locks[1]);
            // if slurp failed we need to add it back
            left_ants[hillNo]++;
            pthread_mutex_unlock(&locks[1]);
        }
    } else {
        pthread_mutex_unlock(&locks[0]);
    }
}

static void *sleep_for_second(void *semaphore) {
    // put current thread to sleep for a second
    if (nanosleep(&t1, &t2) != 0) {
        perror("nanosleep error");
    }
    // post the currentsemaphore so that other aardvark can use it
    if (sem_post((sem_t *) semaphore) != 0) perror ("sem_post error");
}

void *aardvark(void *input) { 
    char aname = *(char *)input; // name of aardvark

    /* INITIALIZATION BY FIRST THREAD */
    pthread_mutex_lock(&init_lock); 
    if (!initialized++) { 
        for (i = 0; i < ANTHILLS; i++) {
            if (sem_init(&semaphores[i], 0, AARDVARKS_PER_HILL) != 0) perror("sem_init error");
            if (pthread_mutex_init(&locks[i], NULL) != 0) perror("pthread_mutex_init error");
            left_ants[i] = ANTS_PER_HILL;
        }
        t1.tv_sec = 1;
        t2.tv_nsec = 600000000;
    } 
    pthread_mutex_unlock(&init_lock); 

    // now be an aardvark
    while (chow_time()) {
        pthread_mutex_lock(&locks[2]);
        // random pick an anthill for current aardvark
        int anthill = rand() % ANTHILLS;
        // if the current anthill do not have three aardvark, eat at here
        if (sem_trywait(&semaphores[anthill]) != -1) {
            pthread_t thread;
            pthread_create(&thread, NULL, sleep_for_second, &semaphores[anthill]);
            pthread_mutex_unlock(&locks[2]);           
            slurp_with_lock(aname, anthill);
            pthread_join(thread, NULL);
        } else {
        // else we just unlock and continue
            pthread_mutex_unlock(&locks[2]);
        }
    } 

    // delete to prevent the memory leak
    // delete at the last aardvark
    pthread_mutex_lock(&init_lock); 
    if (--initialized == AARDVARKS) {
        for (i = 0; i < ANTHILLS; i++) {
            sem_destroy(&semaphores[i]);
            pthread_mutex_destroy(&locks[i]);
        }
    }
    pthread_mutex_unlock(&init_lock); 
    return NULL; 
} 