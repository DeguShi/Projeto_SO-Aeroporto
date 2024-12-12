#ifndef SEMAFORO_H
#define SEMAFORO_H

#include <pthread.h>

class Semaforo {
public:
    Semaforo(int count = 0) : count(count) {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
    }

    ~Semaforo() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    void wait() {
        pthread_mutex_lock(&mutex);
        while(count <= 0) {
            pthread_cond_wait(&cond, &mutex);
        }
        count--;
        pthread_mutex_unlock(&mutex);
    }

    void notificar() {
        pthread_mutex_lock(&mutex);
        count++;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

private:
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

#endif
