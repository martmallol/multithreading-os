#include "barrera.h"

Barrera::Barrera(int x) : N(x) {
    sem_init(&this->b1,0,0);
    sem_init(&this->b2,0,0);
    //this->N = x;
}

void Barrera::wait() {
    // Primera fase
    this->mtx.lock();
    this->n++;
    if (this->n == this->N) {
        // Hago n signals a b1
        for(int i = 0; i < N; i++) {
            sem_post(&this->b1);
        }
    }
    this->mtx.unlock();
    
    sem_wait(&this->b1);

    // Segunda fase
    this->mtx.lock();
    n--;
    cout << "Faltan " << n << " para liberar la barrera" << endl;
    if (n == 0) {
        // Hago n signals a b2
        for(int i = 0; i < N; i++) {
            sem_post(&this->b2);
        }
    }
    this->mtx.unlock();

    sem_wait(&this->b2);
    
}