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
	sem_init(&turno_rojo,0,1);
	sem_init(&turno_azul,0,0); 
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

coordenadas gameMaster::pos_contraria(color equipo){
	if(equipo == ROJO){
		return pos_bandera_azul;
	} else{
		return pos_bandera_roja;
	}
}

void gameMaster::mover_jugador_tablero(coordenadas pos_anterior, coordenadas pos_nueva, color colorEquipo){
    assert(es_color_libre(tablero[pos_nueva.first][pos_nueva.second]));
    tablero[pos_anterior.first][pos_anterior.second] = VACIO; 
    tablero[pos_nueva.first][pos_nueva.second] = colorEquipo;
}


int gameMaster::mover_jugador(direccion dir, int nro_jugador) {
	// Chequear que la movida sea valida
	// Que no se puedan mover 2 jugadores a la vez
    // setear la variable ganador
    // Devolver acorde a la descripción
	lock_guard<mutex> lock(mtx); //hace un mtx.lock()
	//zona crítica
	if(quantum > 0 || quantum == -1) {
		moviendose++;
		//color color = this->turno;
		coordenadas posAnterior; bool noHayNadie; bool sePuedeMoverAhi; bool gano; // indefinidas por ahora
		
		posAnterior = (this->turno == ROJO) ? pos_jugadores_rojos[nro_jugador] : 
										pos_jugadores_azules[nro_jugador];

		coordenadas posProxima = proxima_posicion(posAnterior, dir);
		noHayNadie = es_color_libre(tablero[posProxima.first][posProxima.second]); 
		sePuedeMoverAhi = es_posicion_valida(posProxima) && noHayNadie;
		
		if(sePuedeMoverAhi) {
			// sleep(1);
			mover_jugador_tablero(posAnterior, posProxima, this->turno);
			(this->turno == ROJO) ? pos_jugadores_rojos[nro_jugador] = posProxima : 
							pos_jugadores_azules[nro_jugador] = posProxima;
			cout << "mover_jugador: SOY EL JUGADOR "<< nro_jugador <<" DEL EQUIPO " << this->turno << " Y ME MOVI A (" << posProxima.first << ", " << posProxima.second << ")" << endl;
		} else {
			// CHEQUEAMOS EL GANADOR DOS VECES: UNA EN EL ELSE Y OTRA DESPUES, CAMBIAR ESO, REDUNDANCIA
			int banderaContraria = (this->turno == ROJO) ? BANDERA_AZUL : BANDERA_ROJA;
			if(tablero[posProxima.first][posProxima.second] == banderaContraria) {
				cout << "mover_jugador: LLEGUE A LA BANDERA, GANE!!!!!!!" << endl;
			} else {
				cout << "mover_jugador: SOY EL JUGADOR "<< nro_jugador <<" DEL EQUIPO " << this->turno << " Y ME CHOQUE CON UNA PARED O CON OTRA PERSONA" << endl;
			}
			
		}

		// Chequear ganador
		gano = (this->turno == ROJO) ? (posProxima == pos_bandera_azul) : (posProxima == pos_bandera_roja);
		if (gano) ganador = this->turno;
		if (quantum > 0) quantum--;
		moviendose--;
		return (gano ? 0 : nro_ronda); // "devuelve el nro de ronda o 0 si el equipo gano"
	} else {
		return nro_ronda;
	}
	//se llama al mtx.unlock() automaticamente
}

void gameMaster::termino_ronda(color equipo) {
	//sleep(1);
	cout << "termino_ronda: LISTO PARA TERMINAR LA RONDA" << endl;
	// FIXME: Hacer chequeo de que es el color correcto que está llamando
	// FIXME: Hacer chequeo que hayan terminado todos los jugadores del equipo o su quantum (via mover_jugador)
	// (equipo == ROJO) ? sem_wait(&turno_rojo) : sem_wait(&turno_azul);
	if(equipo == turno && quantum <= 0) {	
		string printEquipo = (equipo == ROJO) ? "rojo" : "azul";
		//busy waiting
		while(moviendose > 0) {
			cout << "termino_ronda: Jugador del equipo " << printEquipo << ": Esperando que terminen mis compas" << endl;
		};
		cout << "termino_ronda: Jugador del equipo " << printEquipo << ": Termino la ronda numero: " << nro_ronda << endl << endl;;
		this->turno = (equipo == ROJO) ? AZUL : ROJO;
		quantum = quantumsOriginales[turno];
		stratActual = strats[turno];
		nro_ronda++;
	}
	 (equipo == ROJO) ? sem_post(&turno_azul), semAzul++ : sem_post(&turno_rojo), semRojo++;
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