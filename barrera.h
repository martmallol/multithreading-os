#ifndef BARRERA_H
#define BARRERA_H

#include <mutex>
#include <semaphore.h>
#include <atomic>
#include <vector>
#include <thread>
#include "definiciones.h"


using namespace std;

class Barrera {
	private:
		// Atributos Privados 
		atomic_int n = 0;
        atomic_int N;
        mutex mtx;
        sem_t b1;
		sem_t b2;
        
	public:
		// Metodos publicos
		Barrera(int x);
		void wait();

};

#endif // BARRERA_H