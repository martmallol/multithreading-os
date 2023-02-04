#include <iostream>
#include <thread>
#include <sys/unistd.h>
#include <time.h> //agregamos librería para generar randoms distintos
#include <vector>
#include <mutex>
#include "gameMaster.h"
#include "equipo.h"
#include "definiciones.h"
#include "config.h"

using namespace std;

const estrategia strat = USTEDES;

int main() {
    
    Config config = *(new Config());

	
    gameMaster belcebu = gameMaster(config);
 

	// Creo equipos (lanza procesos)
	int quantum = 5;
	Equipo rojo(&belcebu, ROJO, strat, config.cantidad_jugadores, quantum, config.pos_rojo);
	Equipo azul(&belcebu, AZUL, strat, config.cantidad_jugadores, quantum, config.pos_azul);
    
    
    rojo.comenzar();
    azul.comenzar();
	rojo.terminar();
	azul.terminar();	
    //belcebu.play();

    if(belcebu.ganador == ROJO || belcebu.ganador == AZUL){
        cout << "Bandera capturada por el equipo "<< belcebu.ganador << ". Felicidades!" << endl;
    } else{
        cout << "Juego finalizado en empate" <<endl;
    }
}