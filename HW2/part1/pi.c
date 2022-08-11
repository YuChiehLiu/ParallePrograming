#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

void* CircleNumsCunt(void* TossesNumsVoid);

long long resSum = 0;
pthread_mutex_t mutex;

int main(int argc, char* argv[]){
    int ThreadNums = atoi(argv[1]);
    char* eptr;
    long long TossesNums = strtoll(argv[2], &eptr, 10);
    long long ThreadTossesNums;
    pthread_t pthread_id[ThreadNums];
    int i=0, j=0, k=0;
    double  pi_estimate;
    ThreadTossesNums = TossesNums / ThreadNums ;

    pthread_mutex_init(&mutex, NULL);



    while(i<ThreadNums){
        pthread_create (&pthread_id[i], NULL, CircleNumsCunt,(void*) &ThreadTossesNums);
        i++;
    }

    srand(time(NULL));

    while(j<ThreadNums){
        pthread_join (pthread_id[j], NULL);
        j++;
    }


    pi_estimate = 4 * (double)resSum / TossesNums ;
    printf("%f\n", pi_estimate);
    
    pthread_mutex_destroy(&mutex);

    return 0;

}

void* CircleNumsCunt(void* TossesNumsVoid){
    double x, y, distance_squared;
    long long cunt;
    unsigned int seed=rand();

    for ( long long toss = 0 ; toss < *(long long*)TossesNumsVoid ; toss++) {
        x = (double) rand_r(&seed) / RAND_MAX;
        y = (double) rand_r(&seed) / RAND_MAX;
        distance_squared = x * x + y * y;
        if ( distance_squared <= 1)
            cunt++;
    }

    pthread_mutex_lock(&mutex);
    resSum += cunt;
    pthread_mutex_unlock(&mutex);

    return NULL;
}