#include "equipo.h"


direccion Equipo::apuntar_a(coordenadas pos1, coordenadas pos2) {
	if (pos2.first > pos1.first) return ABAJO;
	if (pos2.first < pos1.first) return ARRIBA;
	if (pos2.second > pos1.second) 
		return DERECHA;
	else (pos2.second < pos1.second) 
		return IZQUIERDA;
}


void Equipo::jugador(int nro_jugador) {
	//
	// ...
	//

	while(!this->belcebu->termino_juego()) { // Chequear que no haya una race condition en gameMaster
		switch(this->strat) {
			//SECUENCIAL,RR,SHORTEST,USTEDES
			case(SECUENCIAL):
				//
				// ...
				//
				break;
			
			case(RR):
				//
				// ...
				//
				break;

			case(SHORTEST):
				//
				// ...
				//
				break;

			case(USTEDES):
				//
				// ...
				//
				break;
			default:
				break;
		}	
		// Termino ronda ? Recordar llamar a belcebu...
		// OJO. Esto lo termina un jugador... 
		//
		// ...
		//
	}
	
}

Equipo::Equipo(gameMaster *belcebu, color equipo, 
		estrategia strat, int cant_jugadores, int quantum, vector<coordenadas> posiciones) {
	this->belcebu = belcebu;
	this->equipo = equipo;
	this->contrario = (equipo == ROJO)? AZUL: ROJO;
	this->bandera_contraria = (equipo==ROJO)? BANDERA_AZUL: BANDERA_ROJA;
	this->strat = strat;
	this->quantum = quantum;
	this->quantum_restante = quantum;
	this->cant_jugadores = cant_jugadores;
	this->posiciones = posiciones;
	//
	// ...
	//
}

void Equipo::comenzar() {
	// Arranco cuando me toque el turno 
	// TODO: Quien empieza ? 
	//
	// ...
	//
	
	// Creamos los jugadores
	for(int i=0; i < cant_jugadores; i++) {
		jugadores.emplace_back(thread(&Equipo::jugador, this, i)); 
	}
}

void Equipo::terminar() {
	for(auto &t:jugadores){
		t.join();
	}	
}

coordenadas Equipo::buscar_bandera_contraria() {
	//
	// ...
	//
}
