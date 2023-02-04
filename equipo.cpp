#include <sys/unistd.h>
#include <stdio.h>
#include <time.h>
#include "equipo.h"

#define BILLION 1E9

direccion Equipo::apuntar_a(coordenadas pos1, coordenadas pos2) {
	if (pos2.second > pos1.second) return ABAJO;
	if (pos2.second < pos1.second) return ARRIBA;
	if (pos2.first > pos1.first) 
		return DERECHA;
	else if (pos2.first < pos1.first) 
		return IZQUIERDA;
}

void Equipo::jugador(int nro_jugador) { 
	if (equipo == AZUL) sem_wait(&belcebu->turno_azul);
	cout << "Empieza a correr el thread del jugador " << nro_jugador <<endl;
	
	//Código para calcular tiempo en encontrar la bandera contraria
	struct timespec start, finish, delta;
	mtxEquipo.lock();
	if(tiempoInicio == -1){
		tiempoInicio = nro_jugador;
		cout << "Soy el jugador " << nro_jugador << " y le doy inicio al timer para buscar la bandera "<< (equipo + 1) % 2 << endl;
		clock_gettime(CLOCK_REALTIME, &start);
	}
	mtxEquipo.unlock();
	this->buscar_bandera_contraria();
	clock_gettime(CLOCK_REALTIME, &finish);
	mtxEquipo.lock();
	if(tiempoBusqueda == 0){
		tiempoBusqueda = ( finish.tv_sec - start.tv_sec ) + ( finish.tv_nsec - start.tv_nsec ) / BILLION;
		cout << "Jugador " << nro_jugador << ": El tiempo tomado en encontrar la bandera " << (equipo + 1) % 2 << " con los threads fue " << tiempoBusqueda << endl;
	}

	mtxEquipo.unlock();
	cout << "Bandera contraria: "<< this->pos_bandera_contraria.first << ", " << this->pos_bandera_contraria.second << endl;
	
	//SECCION ESTRATEGIAS
	while(!this->belcebu->termino_juego()) { 
					
		// BARRERA
		// Si no se incluye el llamado a barrera1 en la linea 198, hay un deadlock: el ultimo que termino de jugar se va a 
		//dormir primero en el semáforo turno_equipo, y el resto se queda esperando en esta barrera por siempre.
		barrera1->wait();
		cout<<"Salimos de la barrera b1"<<endl;

		switch(this->strat) {
			case(SECUENCIAL): {
				mtxEquipo.lock();
				direccion moverA = apuntar_a(posiciones[nro_jugador], this->pos_bandera_contraria);
				if(!belcebu->termino_juego()){
					chequearMovida(moverA, nro_jugador); // Chequeo si me puedo mover o no
					if(sePuedeMoverAhi) {
						if(belcebu->mover_jugador(moverA, nro_jugador) < 0) {
							cout << "case_secuencial: ERROR EN mover_jugador " << endl;
						} else{
							posiciones[nro_jugador] = posProxima;
						}
					} else{
						cout << "SOY EL JUGADOR "<< nro_jugador <<" DEL EQUIPO " << equipo << " Y ME CHOQUE CON UNA PARED O CON OTRA PERSONA" << endl;
					}
				}
				mtxEquipo.unlock();
				
			} break;
			
			case(RR): {
				cout<<"comienzo RR"<<endl;
				// "Deberán jugar en órden según su número de jugador nro_jugador."
				mtxRR.lock();
				
				//Verificación para reiniciar la ronda circular y para ver si se movió algun jugador durante la ronda
				reiniciarRonda = (cant_jugadores_que_ya_jugaron == cant_jugadores);
				noJugoNadie = (reiniciarRonda && seMovio == vectorIni);
				if(noJugoNadie) quantum_restante = 0;
				if(reiniciarRonda) inicializarVector(), cant_jugadores_que_ya_jugaron = 0;
				if(!empezoRonda) empezoRonda = true, inicializarVector_movidas();
				
				//Validación del jugador actual para ver si es al que le toca moverse
				primJugadorValido = (nro_jugador == 0) ? (yaJugo == vectorIni) : false;
				ultJugadorValido = (nro_jugador == cant_jugadores - 1) ? ((cant_jugadores_que_ya_jugaron == cant_jugadores - 1) && !yaJugo[nro_jugador]) : false;
				niElPrimeroNiElUltimo = (cant_jugadores_que_ya_jugaron == nro_jugador) && (nro_jugador != 0) && (nro_jugador != cant_jugadores-1);
				xJugadorValido = (niElPrimeroNiElUltimo) ? (yaJugo[nro_jugador-1] && !yaJugo[nro_jugador] && !yaJugo[nro_jugador+1]) : false;
				
				if((primJugadorValido || ultJugadorValido || xJugadorValido) && quantum_restante > 0 && !belcebu->termino_juego()) {
					// Chequeo si puedo moverme
					direccion moverA = apuntar_a(posiciones[nro_jugador], pos_bandera_contraria);
					chequearMovida(moverA, nro_jugador); // Chequeo si me puedo mover o no
					
					// Me muevo si puedo
					if(sePuedeMoverAhi) {
						if(belcebu->mover_jugador(moverA, nro_jugador) < 0) {
							cout << "case_RR: ERROR EN mover_jugador " << endl;
						} else {
							posiciones[nro_jugador] = posProxima;
							if(!seMovio[nro_jugador]) seMovio[nro_jugador] = true;
						}
					} else{
						cout << "SOY EL JUGADOR "<< nro_jugador <<" DEL EQUIPO " << equipo << " Y ME CHOQUE CON UNA PARED O CON OTRA PERSONA" << endl;
					}
					mtxRR.unlock();

					//Actualización de variables y estructuras correspondientes post intento de movida
					quantum_restante = belcebu->getQuantumActual();
					cout << "case_RR: Soy el jugador " << nro_jugador << " y el quantum actual es:  " << quantum_restante << endl;
					yaJugo[nro_jugador] = true;
					cant_jugadores_que_ya_jugaron++;
				} else {
					mtxRR.unlock();
				}
				cout << "Me voy ("<<nro_jugador<<")"<<endl;
			} break;

			case(SHORTEST): {
				// Calculamos jugador a menor distancia de bandera contraria. Los demas salen, él se mueve.
				// Mutexeamos esto para que lo calcule solo el primer thread que entra en cada ronda por equipo
				mtxSDF.lock();
				if (jugador_mas_cerca == -1) {
					cout << "Soy el jugador " << nro_jugador << " y buscaré al mas cercano a la bandera" << endl;
					jugador_mas_cerca = shortestDistancePlayer();

					cout << "El jugador mas cercano a la bandera contraria es " << jugador_mas_cerca << endl;
					cout << "PRUEBA QUE SOS EL MÁS CERCANO o SERÁS CASTIGADO!" << endl;
					for (int i = 0; i < cant_jugadores; i++) {
						int distancia_i = belcebu->distancia(posiciones[i], this->pos_bandera_contraria);
						cout << "Distancia a la bandera del jugador " << i << ": " << distancia_i << endl;
					}
				}
				mtxSDF.unlock();
				if (nro_jugador == jugador_mas_cerca) {
					cout << "Soy el jugador " << jugador_mas_cerca << ", el mas cerca a la bandera!" << endl;
					direccion moverA = apuntar_a(posiciones[nro_jugador], pos_bandera_contraria);
					chequearMovida(moverA, nro_jugador); 
					
					// Me muevo si puedo
					if(sePuedeMoverAhi) {
						if(belcebu->mover_jugador(moverA, nro_jugador) < 0) {
							cout << "case_SHORTEST: ERROR EN mover_jugador " << endl;
						} else {
							posiciones[nro_jugador] = posProxima;
						}
					} else{
						cout << "SOY EL JUGADOR "<< nro_jugador <<" DEL EQUIPO " << equipo << " Y ME CHOQUE CON UNA PARED O CON OTRA PERSONA" << endl;
					}
				}
			} break;

			case(USTEDES): {
				//Sección critica para que solo al iniciar la ronda se calcule el vector de jugadoresLejanos
				mtxUDS.lock();
				if (jugadoresLejanos.size() == 0){
					cout << "Soy jugador " << nro_jugador << " y soy quien calcula el vector"<<endl;
					jugadoresLejanos = largestDistancePlayer();
					cantidad = jugadoresLejanos.size();
				}
				for(int i = 0; i < cantidad; i++){
					cout << "Jugador: "<< jugadoresLejanos[i].second << ", pasos: " << jugadoresLejanos[i].first << endl;
				}
				mtxUDS.unlock();

				//Sección crítica: cada jugador del vector de mas lejanos debe hacer todos sus p<=q pasos
				//seguidamente. Pero en principio no por orden de lejania a la bandera.
				mtxUDS.lock();
				cout << "Entra jugador " << nro_jugador << endl;

				noJugoNadie = (cant_jugadores_que_ya_jugaron == jugadoresLejanos.size() && seMovio == vectorIni);
				if(noJugoNadie) quantum_restante = 0;
				if(cant_jugadores_que_ya_jugaron == jugadoresLejanos.size()) cant_jugadores_que_ya_jugaron = 0;
				if(!empezoRonda) empezoRonda = true, inicializarVector_movidas();

            	int cant = 0;
				if (encontrarJugador(jugadoresLejanos, cant, nro_jugador) && quantum_restante > 0 && !belcebu->termino_juego()) {
					cout << "Soy jugador " << nro_jugador << " y me tocan " << cant << " pasos" <<endl;
					cant_jugadores_que_ya_jugaron++;
					for (int i = 0; i < cant; i++) {
						direccion moverA = apuntar_a(posiciones[nro_jugador], this->pos_bandera_contraria);
						chequearMovida(moverA, nro_jugador);
						// Me muevo si puedo
						if(sePuedeMoverAhi) {
							if(belcebu->mover_jugador(moverA, nro_jugador) < 0) {
								cout << "case_RR: ERROR EN mover_jugador " << endl;
							} else {
								posiciones[nro_jugador] = posProxima;
								if(!seMovio[nro_jugador]) seMovio[nro_jugador] = true;
							}
						} else{
							cout << "SOY EL JUGADOR "<< nro_jugador <<" DEL EQUIPO " << equipo << " Y ME CHOQUE CON UNA PARED O CON OTRA PERSONA" << endl;
						}
					}
					mtxUDS.unlock();
					quantum_restante = belcebu->getQuantumActual();
					cout << "case_UDS: Soy el jugador " << nro_jugador << " y el quantum actual es:  " << quantum_restante << endl;
            	} else {
					mtxUDS.unlock();
                	cout << "Soy el jugador " << nro_jugador << ", estoy mas cerca, me voy" << endl;
            	}
            } break;

			default:
				break;
		}	

		barrera1->wait();
		
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
			cout << "Jugador " << nro_jugador << ": Entro a dormirme" <<endl;
			mtxEquipo.lock();
			// Si soy el ultimo en irme a dormir, termino ronda
			if(belcebu->dormidos[equipo] == cant_jugadores-1) {
				mtxEquipo.unlock();
				cout << "Soy el jugador nro " << nro_jugador << " y voy a terminar la ronda" << endl;
				belcebu->termino_ronda(equipo);
				tiempoInicio = -1, tiempoBusqueda = 0; // Reestablezco tiempos para que el otro equipo también la busque
				empezoRonda = false; 
				quantum_restante = quantum; // Reestablezco quantum en caso de RR o USTEDES
				jugador_mas_cerca = -1; // Reestablezco jugador_mas_cerca para que lo recalcule en la prox ronda
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

void Equipo::chequearMovida(direccion moverA, int nro_jugador) {
	//Verificamos que la posición proxima a moverse está en los limites del tablero y no hay nadie allí
	posProxima = belcebu->proxima_posicion(posiciones[nro_jugador], moverA);
	noHayNadie = belcebu->es_color_libre(belcebu->en_posicion(posProxima)) || pos_bandera_contraria == posProxima; 
	sePuedeMoverAhi = belcebu->es_posicion_valida(posProxima) && noHayNadie;
}

void Equipo::meDuermo() {
	if (equipo == ROJO) {
		cout << "Me duermo rojo" << endl, sem_wait(&belcebu->turno_rojo);
	} else {
		cout << "Me duermo azul" << endl, sem_wait(&belcebu->turno_azul);
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
	this->seMovio.resize(cant_jugadores,false);

	this->barrera1 = new Barrera(cant_jugadores);
	this->jugador_mas_cerca = -1;
	this->empezoRonda = false;
	this->jugadoresLejanos.resize(0);

	cout << "SE HA INICIALIZADO EQUIPO " << equipo << " CON EXITO " << endl;
}

void Equipo::comenzar() {
	if(equipo){cout << "SOY DEL EQUIPO ROJO"<< endl;} 
	else{cout << "SOY DEL EQUIPO AZUL"<<endl;}
	
	//Chequeamos tiempo en buscar la bandera de forma secuencial
	struct timespec start, finish, delta;
	clock_gettime(CLOCK_REALTIME, &start);
	this->buscar_bandera_contraria_secuencial();
	clock_gettime(CLOCK_REALTIME, &finish);

	double accum = ( finish.tv_sec - start.tv_sec ) + ( finish.tv_nsec - start.tv_nsec ) / BILLION;
	cout << "El tiempo tomado en encontrar la bandera " << (equipo + 1) % 2 << " de forma secuencial fue " << accum << endl;

	cout << "Bandera contraria secuencial: "<< this->pos_bandera_contraria.first << ", " << this->pos_bandera_contraria.second << endl;

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

// BUSQUEDA BANDERA MEDIANTE RECORRIDA DE POSICIONES DEL TABLERO, REQUERIDA PARA IMPLEMENTAR ESTRATEGIAS EN PUNTO 3
coordenadas Equipo::buscar_bandera_contraria_secuencial() {
 bool encontrada;
 for (int i = 0; i < this->belcebu->getTamx() && !encontrada; i++) {
	for (int j = 0; j < this->belcebu->getTamy() && !encontrada; j++) {
		color banderaContraria = (color)(this->equipo + 5);
		encontrada = this->belcebu->en_posicion({i, j}) == banderaContraria;
		if (encontrada) {
			this->pos_bandera_contraria = {i, j};
		}
	}
 }
 return this->pos_bandera_contraria;
}

// PUNTO 4 -- BUSCAMOS BANDERA CONTRARIA DANDOLE AL PRIMER THREAD EN LLEGAR UNA FILA AÚN NO RECORRIDA
coordenadas Equipo::buscar_bandera_contraria() {
	int totalFilas = this->belcebu->getTamx();
	while(!encontramosBandera && filaNoRecorrida < totalFilas){
		mtxBandera.lock();
		filaNoRecorrida.fetch_add(1);
		recorrerFila(filaNoRecorrida - 1);
	}
}

//RECORREMOS UNA FILA 
void Equipo::recorrerFila(int fila){
	int totalFilas = this->belcebu->getTamx();
	mtxBandera.unlock();
	if(fila < totalFilas){
	color banderaContraria = (color)(this->equipo + 5);
		for(int j = 0; j < this->belcebu->getTamy() && !encontramosBandera; j++){
			encontramosBandera = this->belcebu->en_posicion({fila, j}) == banderaContraria;
			if (encontramosBandera) {
				cout << "Hallamos bandera" << endl;
				this->pos_bandera_contraria = {fila, j};
			}
		}
	}
}

// FUNCIONES PARA RR
//Inicializamos vectores de yaJugo y seMovio
void Equipo::inicializarVector() {
	for (int i = 0; i < cant_jugadores; i++) {
		yaJugo[i] = false;
	}
}

void Equipo::inicializarVector_movidas() {
	for (int i = 0; i < cant_jugadores; i++) {
		seMovio[i] = false;
	}
}

// FUNCION PARA SHORTEST DISTANCE PLAYER
//Calculamos jugador más cercano a la bandera
int Equipo::shortestDistancePlayer() {
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

// FUNCIONES PARA ESTRATEGIA PROPIA
vector<pair<int, int>> Equipo::largestDistancePlayer() {
	int q = belcebu->getQuantumActual();
	vector<pair<int, int>> jugadores_Lejanos; //creamos vector de pares, cada par es (quantum_jugador, jugador)
	while (q != 0 && jugadores_Lejanos.size() < cant_jugadores) {
		//Mientras haya quantum que dividir y jugadores disponibles para asignarselo, lo dividimos en potencias de 2 y 
		//cargamos un par en el vector al que falta asignarle a que jugador le pertenecerá ese quantum
		if(q > 1){
			jugadores_Lejanos.push_back(make_pair(q/2, -1));
			q -= q/2;
		} else{
			jugadores_Lejanos.push_back(make_pair(q%2, -1));
			q -= q%2;
		} 
	}

	int jugador = 0;
	int distancia_max = belcebu->distancia(posiciones[jugador], this->pos_bandera_contraria);
	//Recorremos cada posición del vector de pares recién creados.
	//En la primer posición ponemos como jugador en (x,y).second al jugador más lejano a la bandera, en la segunda
	//al segundo más lejano, etc..
	for (int j = 0; j < jugadores_Lejanos.size(); j++) {
		for (int i = 0; i < cant_jugadores; i++) {
			if(!in_vector(jugadores_Lejanos, i)){
				if (belcebu->distancia(posiciones[i], this->pos_bandera_contraria) > distancia_max) {
					jugador = i;
					distancia_max = belcebu->distancia(posiciones[i], this->pos_bandera_contraria);
				}
			}
		}
		jugadores_Lejanos[j].second = jugador;
		distancia_max = 0;
	}
	return jugadores_Lejanos;
}

bool Equipo::encontrarJugador(vector<pair<int, int>> jugadores_Lejanos, int &cant, int nro_jugador) {
	//Función llamada por cada jugador para saber si le toca moverse y en ese caso, se devuelve no solo un true,
	//sino una variable cant por referencia que indica el quantum específico que tiene
	cant = 0;
	for (int i = 0; i < jugadores_Lejanos.size(); i++) {
		if (jugadores_Lejanos[i].second == nro_jugador) {
			cant = jugadores_Lejanos[i].first;
			return true;
		}
	}
	return false;
}

//Función para ver si un jugador ya estaba cargado en el vector
bool Equipo::in_vector(vector<pair<int, int>> jugadores_Lejanos, int nro_jugador) {
	for (int i = 0; i < jugadores_Lejanos.size(); i++) {
		if (jugadores_Lejanos[i].second == nro_jugador)
			return true;
		}
	return false;
}