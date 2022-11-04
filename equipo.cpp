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
		// deadlock, el ultimo que termino de jugar se va a dormir primero,
		// y el resto se queda esperando aca por siempre 
		barrera1->wait();
		printf("Salimos de la barrera b1\n");

		switch(this->strat) {
			//SECUENCIAL,RR,SHORTEST,USTEDES
			case(SECUENCIAL):
				if(belcebu->getTurno() == equipo) {
					// cout << "Mi posicion es: " << posiciones[nro_jugador].first << ", " << posiciones[nro_jugador].second << endl;
					// cout << "La bandera del otro equipo esta en " << this->pos_bandera_contraria.first << ", " << this->pos_bandera_contraria.second << endl;
					direccion moverA = apuntar_a(posiciones[nro_jugador], this->pos_bandera_contraria);
					if(belcebu->mover_jugador(moverA, nro_jugador) < 0) {
						cout << "case_secuencial: ERROR EN mover_jugador " << endl;
					} 
				}
				break;
			
			case(RR):
				printf("comienzo RR\n");
				// "Deberán jugar en órden según su número de jugador nro_jugador."
				mtxRR.lock();
				reiniciarRonda = (cant_jugadores_que_ya_jugaron == cant_jugadores);
				if(reiniciarRonda) inicializarVector(), cant_jugadores_que_ya_jugaron = 0;
				primJugadorValido = (nro_jugador == 0) ? (yaJugo == vectorIni) : false;
				ultJugadorValido = (nro_jugador == cant_jugadores - 1) ? ((cant_jugadores_que_ya_jugaron == cant_jugadores - 1) && !yaJugo[nro_jugador]) : false;
				niElPrimeroNiElUltimo = (cant_jugadores_que_ya_jugaron == nro_jugador) && (nro_jugador != 0) && (nro_jugador != cant_jugadores-1);
				xJugadorValido = (niElPrimeroNiElUltimo) ? (yaJugo[nro_jugador-1] && !yaJugo[nro_jugador] && !yaJugo[nro_jugador+1]) : false;
				if(primJugadorValido || ultJugadorValido || xJugadorValido) {
					mtxRR.unlock();
					printf("voy a moverme\n");
					direccion moverA = apuntar_a(posiciones[nro_jugador], pos_bandera_contraria);
					if(belcebu->mover_jugador(moverA, nro_jugador) < 0) {
						cout << "case_RR: ERROR EN mover_jugador " << endl;
					}
					// Informativo
					quantum_restante = belcebu->getQuantumActual();
					cout << "case_RR: Soy el jugador " << nro_jugador << " y el quantum actual es:  " << quantum_restante << endl;
					yaJugo[nro_jugador] = true;
					cant_jugadores_que_ya_jugaron++;
				} else {
					mtxRR.unlock();
				}
				printf("Me voy\n");
				break;

			case(SHORTEST):
				// Calculamos jugador a menor distancia de bandera contraria. Los demas ¿se duermen?, él se mueve.
				// Mutexeo esto para que lo calcule solo el primer thread que entra en cada ronda por equipo
				mtxSDF.lock();
				if (jugador_mas_cerca == -1) {
					cout << "Soy el jugador " << nro_jugador << " y buscaré al mas cercano a la bandera" << endl;
					jugador_mas_cerca = shortestDistancePlayer();

					cout << "El jugador mas cercano a la bandera contraria es " << jugador_mas_cerca << endl;
					cout << "PRUEBALO o SERÁS CASTIGADO!" << endl;
					for (int i = 0; i < cant_jugadores; i++) {
						int distancia_i = belcebu->distancia(posiciones[i], this->pos_bandera_contraria);
						cout << "Distancia a la bandera del jugador " << i << ": " << distancia_i << endl;
					}
				}
				mtxSDF.unlock();
				if (nro_jugador == jugador_mas_cerca) {
					cout << "Soy el jugador " << jugador_mas_cerca << ", el mas cerca a la bandera!" << endl;
					direccion moverA = apuntar_a(posiciones[nro_jugador], this->pos_bandera_contraria);
					int mov_jugador = belcebu->mover_jugador(moverA, nro_jugador);
				} else {
					cout << "Soy el jugador " << nro_jugador << ", estoy mas lejos, me voy" << endl;
				}
				break;

			case(USTEDES):
				//
				break;
			default:
				break;
		}	

		barrera1->wait();

		// OJO. Esto lo termina un jugador... ELEGIMOS QUE LO HAGA EL ULTIMO EN IRSE A DORMIR
		// Si termino el juego, despierto al perdedor
		if(this->belcebu->termino_juego()) {
			// Despierto a los primeros 'cant_jugadores-1' jugadores del otro equipo
			(equipo == ROJO) ? sem_post(&belcebu->turno_azul), belcebu->dormidos[AZUL]-- : 
			sem_post(&belcebu->turno_rojo), belcebu->dormidos[ROJO]--;

			// Despierto al ultimo
			if(equipo == ROJO && !belcebu->desperteUltimoRonda[AZUL]) {
				sem_post(&belcebu->termina_ronda_azul);
				belcebu->desperteUltimoRonda[AZUL] = true;
			} else if(equipo == AZUL && !belcebu->desperteUltimoRonda[ROJO]) {
				sem_post(&belcebu->termina_ronda_rojo);
				belcebu->desperteUltimoRonda[ROJO] = true;
			}
		}
		// Si termino mi turno, hago el cambio y me voy a dormir
		else if((quantum_restante == 0) || (quantum_restante == -1)) {
			printf("Jugador %d: Entro a la dormicion\n", nro_jugador);
			mtxEquipo.lock();
			// Si soy el ultimo en irme a dormir, termino ronda
			if(belcebu->dormidos[equipo] == cant_jugadores-1) {
				mtxEquipo.unlock();
				cout << "Soy el jugador nro " << nro_jugador << " y voy a terminar la ronda" << endl;
				belcebu->termino_ronda(equipo);
				quantum_restante = quantum; // Reestablezco quantum en caso de RR o USTEDES
				jugador_mas_cerca = -1; // Reestablezco jugador_mas_cerca para que lo recalcule en la prox ronda
			} 
			// Para RR. Quienes sabemos que nunca mas van a volver a jugar, se duermen apneas se levantan
			else if(quantum_restante > nro_jugador) {
				mtxEquipo.unlock();
				while(quantum_restante > nro_jugador) {
					// Nos dormimos
					belcebu->dormidos[equipo]++;
					meDuermo();
				}
			}
			// Caso para todas las estrategias (los que no son quien termina ronda)
			else {
				// Nos dormimos
				belcebu->dormidos[equipo]++;
				mtxEquipo.unlock();
				meDuermo();
			}
		}
	}

	if(this->belcebu->ganador == equipo) cout << "EQUIPO " << equipo << ": ASI, ASI, ASI GANA EL MADRID!" << endl;
	else cout << "EQUIPO " << equipo << ": ES INCREIBLE PERO NO SE NOS DA" << endl;
}

void Equipo::meDuermo() {
	if (equipo == ROJO) {
		cout << "Me duermo rojo" << endl, sem_wait(&belcebu->turno_rojo), belcebu->semRojo--;
	} else {
		cout << "Me duermo azul" << endl, sem_wait(&belcebu->turno_azul), belcebu->semAzul--;
	}
	belcebu->dormidos[equipo]--;
}

Equipo::Equipo(gameMaster *belcebu, color equipo, 
		estrategia strat, int cant_jugadores, int quantum, vector<coordenadas> posiciones) {
	this->belcebu = belcebu;
	this->equipo = equipo;
	this->contrario = (equipo == ROJO)? AZUL: ROJO;
	this->bandera_contraria = (equipo==ROJO)? BANDERA_AZUL: BANDERA_ROJA;
	this->strat = strat;
	// Aclaracion: Decidimos que si la estrategia es secuencial, el quantum tiene que ser -1.
	// Es decir, si quantum = -1, la estrategia NO posee quantum
	this->quantum = (strat != SECUENCIAL && strat != SHORTEST) ? quantum : -1; 
	this->quantum_restante = this->quantum;
	this->cant_jugadores = cant_jugadores;
	this->posiciones = posiciones;

	// Inicializaciones propias
	this->posiciones_originales = posiciones;
	belcebu->setearQuantum(this->quantum,equipo);
	this->yaJugo.resize(cant_jugadores,false);
	this->vectorIni.resize(cant_jugadores,false);
	// POR QUE NO COMPILA SI NO ES UN PUNTERO????
	this->barrera1 = new Barrera(cant_jugadores);
	this->jugador_mas_cerca = -1;

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

// TODO: Hacerla bien
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

// Funcion para RR
void Equipo::inicializarVector(){
	for(int i = 0; i < cant_jugadores; i++) {
		yaJugo[i] = false;
	}
}

// Funcion para SHORTEST DISTANCE PLAYER
int Equipo::shortestDistancePlayer()
{
    int jugador = 0;
    int distancia_min = belcebu->distancia(posiciones[jugador], this->pos_bandera_contraria);
    for (int i = 1; i < cant_jugadores; i++) {
        if (belcebu->distancia(posiciones[i], this->pos_bandera_contraria) < distancia_min) {
            jugador = i;
            distancia_min = belcebu->distancia(posiciones[i], this->pos_bandera_contraria);
        }
    }
    return jugador;
}