#include <sys/unistd.h>
#include "equipo.h"

direccion Equipo::apuntar_a(coordenadas pos1, coordenadas pos2) {
	if (pos2.first > pos1.first) return ABAJO;
	if (pos2.first < pos1.first) return ARRIBA;
	if (pos2.second > pos1.second) 
		return DERECHA;
	else if (pos2.second < pos1.second) 
		return IZQUIERDA;
}

void Equipo::jugador(int nro_jugador) { 
	//
	// ...
	//
	(equipo == ROJO) ? sem_wait(&belcebu->turno_rojo) : sem_wait(&belcebu->turno_azul);
	cout << "jugador: JUGAR MAL FUE PARTE DE LA ESTRATEGIA"<<endl;
	// ACA PODRIA HABER UNA BARRERA
    //this->pos_bandera_contraria = buscar_bandera_contraria();
	while(!this->belcebu->termino_juego()) { // Chequear que no haya una race condition en gameMaster
		switch(this->strat) {
			//SECUENCIAL,RR,SHORTEST,USTEDES
			case(SECUENCIAL):
				//
				//
				//cout << "ESTOY EN SECUENCIAL, SOY DEL " << equipo << " Y SOY EL JUGADOR " << nro_jugador << endl;
				if(!this->yaJugo[nro_jugador] && belcebu->getTurno() == equipo){
					direccion moverA = apuntar_a(posiciones[nro_jugador], belcebu->pos_contraria(equipo));
					if(belcebu->mover_jugador(moverA, nro_jugador) < 0){
						cout << "case_secuencial: ERROR EN mover_jugador " << endl;
					} else{
						cout << "case_secuencial: SOY EL JUGADOR " << nro_jugador << " DEL EQUIPO " << equipo << " Y ME MOVÍ HACIA " << moverA << endl;
						yaJugo[nro_jugador] = true;
						cant_jugadores_que_ya_jugaron++;
						//VER COMO CAMBIAR POSICION DEL JUGADOR 
						//posiciones[nro_jugador] =;						
					}
				}
				
				
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
		if(quantum_restante == 0 || (quantum_restante == -1 && cant_jugadores_que_ya_jugaron == cant_jugadores)) {
			cout << "jugador: Voy a llamar termino_ronda" << endl;
			if(quantum_restante == -1){
				inicializarVector();
				cant_jugadores_que_ya_jugaron = 0;
			}
			belcebu->termino_ronda(equipo);
		} else if (equipo == ROJO) {
			//cout << "DESPIERTO OTRO ROJO " << endl;
			sem_post(&belcebu->turno_rojo);
		} else {
			//cout << "DESPIERTO OTRO AZUL " << endl;
			sem_post(&belcebu->turno_azul);
		}
		
		(equipo == ROJO) ? sem_wait(&belcebu->turno_rojo) : sem_wait(&belcebu->turno_azul);
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
	// Inicializaciones propias
	this->posiciones_originales = posiciones;
	belcebu->setearQuantum(quantum,equipo);
	this->yaJugo.resize(cant_jugadores,false);
	//inicializarVector();
	cout << "SE HA INICIALIZADO EQUIPO " << equipo << " CON EXITO " << endl;
	//
}

void Equipo::comenzar() {
	// Arranco cuando me toque el turno 
	// TODO: Quien empieza ? 
	if(equipo){cout << "SOY DEL EQUIPO ROJO"<< endl;} 
	else{cout << "SOY DEL EQUIPO AZUL"<<endl;}
	//Para ir probando estrategias antes de tener buscar_bandera_contraria hecha, esto funciona aunque ESTA MAL!
	cout << belcebu->pos_contraria(ROJO).first << ", " << belcebu->pos_contraria(ROJO).second << endl;
	cout << belcebu->pos_contraria(AZUL).first << ", " << belcebu->pos_contraria(AZUL).second << endl;
	
	// busy waiting -> esto ya nos asegura que ningún azul comienza antes que los del rojo
	while((belcebu->getTurno() == ROJO) && equipo == AZUL) {}
	cout << "SALI DEL BUSY WAITING " << endl;
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
	//Para ir obteniendo valores distintos con random
	srand(time(NULL));
	direccion random;
	// Seguir buscando mientras no encuentro bandera
	bool encontre = false;
	while(!encontre){
		random = (direccion)(rand() % 4);
		
	}
	
	//TODO: Reestablecer posición original de los jugadores
}

/*bool Equipo::jugaronTodos(){
	for(int i = 0; i < cant_jugadores; i++){
		if(!yaJugo[i]){return false;}
	}
	return true;
}*/

void Equipo::inicializarVector(){
	for(int i = 0; i < cant_jugadores; i++){
		yaJugo[i] = false;
	}
}