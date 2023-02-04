#include <sys/unistd.h>
#include <assert.h>     /* assert */
#include "gameMaster.h"

bool gameMaster::es_posicion_valida(coordenadas pos) {
	return (pos.first > 0) && (pos.first < x) && (pos.second > 0) && (pos.second < y);
}

bool gameMaster::es_color_libre(color color_tablero){
    return color_tablero == VACIO || color_tablero == INDEFINIDO;
}

color gameMaster::en_posicion(coordenadas coord) {
	return tablero[coord.first][coord.second];
}

int gameMaster::getTamx() {
	return x;
}

int gameMaster::getTamy() {
	return y;
}

int gameMaster::distancia(coordenadas c1, coordenadas c2) {
    return abs(c1.first-c2.first)+abs(c1.second-c2.second);
}

gameMaster::gameMaster(Config config) {
	assert(config.x>0); 
	assert(config.y>0); // Tamaño adecuado del tablero

    this->x = config.x;
	this->y = config.y;

	assert((config.bandera_roja.first == 1)); // Bandera roja en la primera columna
	assert(es_posicion_valida(config.bandera_roja)); // Bandera roja en algún lugar razonable

	assert((config.bandera_azul.first == x-1)); // Bandera azul en la ultima columna
	assert(es_posicion_valida(config.bandera_azul)); // Bandera roja en algún lugar razonable

	assert(config.pos_rojo.size() == config.cantidad_jugadores);
	assert(config.pos_azul.size() == config.cantidad_jugadores);
	for(auto &coord : config.pos_rojo) {
		assert(es_posicion_valida(coord)); // Posiciones validas rojas
	}		

	for(auto &coord : config.pos_azul) {
		assert(es_posicion_valida(coord)); // Posiciones validas rojas
	}		

	this->jugadores_por_equipos = config.cantidad_jugadores;
	this->pos_bandera_roja = config.bandera_roja;
	this->pos_bandera_azul = config.bandera_azul;
    this->pos_jugadores_rojos = config.pos_rojo;
    this->pos_jugadores_azules = config.pos_azul;
	// Seteo tablero
	tablero.resize(x);
    for (int i = 0; i < x; ++i) {
        tablero[i].resize(y);
        fill(tablero[i].begin(), tablero[i].end(), VACIO);
    }
    
    for(auto &coord : config.pos_rojo){
        assert(es_color_libre(tablero[coord.first][coord.second])); //Compruebo que no haya otro jugador en esa posicion
        tablero[coord.first][coord.second] = ROJO; // guardo la posicion
    }

    for(auto &coord : config.pos_azul){
        assert(es_color_libre(tablero[coord.first][coord.second]));
        tablero[coord.first][coord.second] = AZUL;
    }

    tablero[config.bandera_roja.first][config.bandera_roja.second] = BANDERA_ROJA;
    tablero[config.bandera_azul.first][config.bandera_azul.second] = BANDERA_AZUL;
	this->turno = ROJO;

    cout << "SE HA INICIALIZADO GAMEMASTER CON EXITO" << endl;
	if(turno){
		cout << "EL TURNO ACTUAL ES ROJO" << endl;
	} else{
		cout << "EL TURNO ACTUAL ES AZUL" << endl;
	}
	
    // Insertar código que crea necesario de inicialización
	sem_init(&turno_rojo,0,0), sem_init(&turno_azul,0,0); 
	sem_init(&termina_ronda_rojo,0,0), sem_init(&termina_ronda_azul,0,0);
}

void gameMaster::setearEstrategia(estrategia strategy, color equipo) {
	strats[equipo] = strategy;
}

void gameMaster::setearQuantum(int q, color equipo) {
	quantumsOriginales[equipo] = q;
	if(turno == equipo){
		quantum = q;
	}
}

color gameMaster::getTurno() {
	return turno;
}

int gameMaster::getQuantumActual() {
	return quantum;
}

void gameMaster::mover_jugador_tablero(coordenadas pos_anterior, coordenadas pos_nueva, color colorEquipo){
    assert(es_color_libre(tablero[pos_nueva.first][pos_nueva.second]));
    tablero[pos_anterior.first][pos_anterior.second] = VACIO; 
    tablero[pos_nueva.first][pos_nueva.second] = colorEquipo;
}

int gameMaster::mover_jugador(direccion dir, int nro_jugador) {
	coordenadas posAnterior; bool gano; // indefinidas por ahora
	posAnterior = (this->turno == ROJO) ? pos_jugadores_rojos[nro_jugador] : 
									pos_jugadores_azules[nro_jugador];
	coordenadas posProxima = proxima_posicion(posAnterior, dir);
	
	//Se mueve directamente
	//Log
	cout << "Pos anterior de jugador "<< nro_jugador << " del equipo " << this->turno <<": ("<<posAnterior.first<<","<<posAnterior.second<<")"<<endl;
	cout << "Pos próxima de jugador "<< nro_jugador << " del equipo " << this->turno <<": ("<<posProxima.first<<","<<posProxima.second<<")"<<endl;
	cout << "Llamo a mover_jugador_tablero ("<< nro_jugador <<")" <<endl;

	//Alguien jugó, verificando posible empate, si vine a la función es porque alguien se mueve
	(this->turno == ROJO) ? movio_alguien_rojo++ : movio_alguien_azul++;

	// Chequeo ganador
	gano = (this->turno == ROJO) ? (posProxima == pos_bandera_azul) : (posProxima == pos_bandera_roja);
	
	//Movimiento
	if(!gano) {	
		mover_jugador_tablero(posAnterior, posProxima, this->turno);
		(this->turno == ROJO) ? pos_jugadores_rojos[nro_jugador] = posProxima : 
					pos_jugadores_azules[nro_jugador] = posProxima;
		cout << "mover_jugador: SOY EL JUGADOR "<< nro_jugador <<" DEL EQUIPO " << this->turno << " Y ME MOVI A (" << posProxima.first << ", " << posProxima.second << ")" << endl;
	} 
	// Ganamos
	else {
		cout << "mover_jugador: LLEGUE A LA BANDERA, GANE!!!!!!!" << endl;
	}
	
	if (gano) ganador = this->turno, this->turno = VACIO;
	if (quantum > 0) quantum--; 
	return (gano ? 0 : nro_ronda); // "devuelve el nro de ronda o 0 si el equipo gano"
}

void gameMaster::termino_ronda(color equipo) {
	
	if(equipo == turno) {	
		string printEquipo = (equipo == ROJO) ? "rojo" : "azul";
		cout << "termino_ronda: Jugador del equipo " << printEquipo << ": Termino la ronda numero: " << nro_ronda << endl << endl;
		this->turno = (equipo == ROJO) ? AZUL : ROJO;
		quantum = quantumsOriginales[turno];
		stratActual = strats[turno];
		nro_ronda++;
	}

	//Me fijo si jugó alguien de algún equipo esta ronda. Caso contrario, hay empate y el juego termina
	if(movio_alguien_rojo == 0 && movio_alguien_azul == 0 && turno == ROJO){
		ganador = EMPATE; 
	} else if(turno == ROJO){
		movio_alguien_rojo = 0;
		movio_alguien_azul = 0;
	} 
	
	// Si es la primera ronda despierto a los del azul que estan todos dormidos
	if (nro_ronda == 1) {
		for (int i = 0; i < jugadores_por_equipos; i++) {
			sem_post(&turno_azul);
		}
		dormidos[equipo]++;
		sem_wait(&termina_ronda_rojo);
	}
	// Me voy a dormir y despierto a quien termino ronda del equipo contrario
	else if(equipo == ROJO) {
		sem_post(&termina_ronda_azul);
		dormidos[equipo]++;
		sem_wait(&termina_ronda_rojo);
	} else {
		sem_post(&termina_ronda_rojo);
		dormidos[equipo]++;
		sem_wait(&termina_ronda_azul);
	}
	
	// Me despierto y levanto a los de mi equipo
	cout<<"termino_ronda: ME DESPERTE!"<<endl;
	dormidos[equipo]--;

	// Si termino el juego, nos despertamos todos los jugadores de ambos equipos
	if(this->termino_juego()) {
		(equipo == ROJO) ? sem_post(&termina_ronda_azul) : sem_post(&termina_ronda_rojo);
	}

	cout<<"termino_ronda: Voy a despertar a mis compas!"<<endl;
	for (int i = 0; i < jugadores_por_equipos-1; i++) {
		(equipo == ROJO) ? sem_post(&turno_rojo): sem_post(&turno_azul);
	}
}

bool gameMaster::termino_juego() {
	return ganador != INDEFINIDO;
}

coordenadas gameMaster::proxima_posicion(coordenadas anterior, direccion movimiento) {
	// Calcula la proxima posición a moverse (es una copia) 
	switch(movimiento) {
		case(ARRIBA):
			anterior.second--; 
			break;

		case(ABAJO):
			anterior.second++;
			break;

		case(IZQUIERDA):
			anterior.first--;
			break;

		case(DERECHA):
			anterior.first++;
			break;
	}
	return anterior; // está haciendo una copia por constructor
}