#include <sys/unistd.h>
#include "equipo.h"

direccion Equipo::apuntar_a(coordenadas pos1, coordenadas pos2) {
	if (pos2.second > pos1.second) return ABAJO;
	if (pos2.second < pos1.second) return ARRIBA;
	if (pos2.first > pos1.first) 
		return DERECHA;
	else if (pos2.first < pos1.first) 
		return IZQUIERDA;
}

void Equipo::jugador(int nro_jugador) { 
	// Como los despertamos? En belcebu->termino_ronda.
	if (equipo == AZUL) sem_wait(&belcebu->turno_azul), belcebu->semAzul--;
	cout << "jugador: JUGAR MAL FUE PARTE DE LA ESTRATEGIA"<<endl;
	
	while(!this->belcebu->termino_juego()) { // Chequear que no haya una race condition en gameMaster
		
		cout << "Dormidos equipo "<< equipo << ": " << belcebu->dormidos[equipo] << endl;
		
		// BARRERA
		barrera1->wait();
		printf("Salimos de la barrera b1\n");

		switch(this->strat) {
			//SECUENCIAL,RR,SHORTEST,USTEDES
			case(SECUENCIAL):
				if(belcebu->getTurno() == equipo) {
					// cout << "Mi posicion es: " << posiciones[nro_jugador].first << ", " << posiciones[nro_jugador].second << endl;
					// cout << "La bandera del otro equipo esta en " << belcebu->pos_contraria(equipo).first << ", " << belcebu->pos_contraria(equipo).first << endl;
					direccion moverA = apuntar_a(posiciones[nro_jugador], this->pos_bandera_contraria);
					if(belcebu->mover_jugador(moverA, nro_jugador) < 0) {
						cout << "case_secuencial: ERROR EN mover_jugador " << endl;
					} 
				}
				break;
			
			case(RR):
				// "Deberán jugar en órden según su número de jugador nro_jugador."
				
				break;

			case(SHORTEST):
				//
				break;

			case(USTEDES):
				//
				break;
			default:
				break;
		}	

		// OJO. Esto lo termina un jugador... ELEGIMOS QUE LO HAGA EL ULTIMO EN IRSE A DORMIR
		if(quantum_restante == 0 || (quantum_restante == -1)) {
			mtxEquipo.lock();
			// Si soy el ultimo en irme a dormir
			if(belcebu->dormidos[equipo] == cant_jugadores-1) {
				mtxEquipo.unlock();
				cout << "Soy el jugador nro " << nro_jugador << " y voy a terminar la ronda" << endl;
				belcebu->termino_ronda(equipo);
			} else {
				// Nos dormimos
				belcebu->dormidos[equipo]++;
				mtxEquipo.unlock();
				if (equipo == ROJO) {
					cout << "Me duermo rojo" << endl, sem_wait(&belcebu->turno_rojo), belcebu->semRojo--;
				} else {
					cout << "Me duermo azul" << endl, sem_wait(&belcebu->turno_azul), belcebu->semAzul--;
				}
				belcebu->dormidos[equipo]--;
			}
		} 
	}

	if(this->belcebu->ganador == equipo) cout << "EQUIPO " << equipo << ": ASI, ASI, ASI GANA EL MADRID!" << endl;
	else cout << "EQUIPO " << equipo << ": ES INCREIBLE PERO NO SE NOS DA" << endl;
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

	// Inicializaciones propias
	this->posiciones_originales = posiciones;
	belcebu->setearQuantum(quantum,equipo);
	this->yaJugo.resize(cant_jugadores,false);

	// POR QUE NO COMPILA SI NO SON PUNTEROS????
	this->barrera1 = new Barrera(cant_jugadores);

	cout << "SE HA INICIALIZADO EQUIPO " << equipo << " CON EXITO " << endl;
}

void Equipo::comenzar() {
	if(equipo){cout << "SOY DEL EQUIPO ROJO"<< endl;} 
	else{cout << "SOY DEL EQUIPO AZUL"<<endl;}
	
	this->buscar_bandera_contraria_secuencial();
	cout << "Bandera contraria: "<< this->pos_bandera_contraria.first << ", " << this->pos_bandera_contraria.second << endl;

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

coordenadas Equipo::buscar_bandera_contraria_secuencial() {
	bool encontrada;
	for (int i = 0; i < this->belcebu->getTamx() && !encontrada; i++) {
		for (int j = 0; j < this->belcebu->getTamy() && !encontrada; j++) {
			color banderaContraria = (color)(this->equipo + 5);
			encontrada = this->belcebu->en_posicion({i,j}) == banderaContraria;
			if(encontrada) {
				this->pos_bandera_contraria = {i,j};
			}
		}
	}
	return this->pos_bandera_contraria;
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