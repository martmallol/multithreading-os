#ifndef EQUIPO_H
#define EQUIPO_H

#include <semaphore.h>
#include <vector>
#include <thread>
#include "definiciones.h"
#include "gameMaster.h"
#include "barrera.h"

using namespace std;

class Equipo {
	private:

		// Atributos Privados 
		gameMaster *belcebu; 
		color contrario, equipo, bandera_contraria;
		estrategia strat;
		int cant_jugadores, quantum, quantum_restante;
		vector<thread> jugadores;
		int cant_jugadores_que_ya_jugaron = 0;
		vector<coordenadas> posiciones;
		coordenadas pos_bandera_contraria;

		//Agregamos atributos
		mutex mtxEquipo;
		mutex mtxRR;
		vector<coordenadas> posiciones_originales;
		vector<bool> yaJugo;
		vector<bool> vectorIni;
		Barrera* barrera1;
		bool reiniciarRonda;
		bool primJugadorValido;
		bool ultJugadorValido;
		bool xJugadorValido;
		bool niElPrimeroNiElUltimo;

		// Métodos privados 
		direccion apuntar_a(coordenadas pos2, coordenadas pos1);
		void jugador(int nro_jugador);
		coordenadas buscar_bandera_contraria();

		// Agregamos métodos privados
		//bool jugaronTodos();
		void inicializarVector();
		coordenadas buscar_bandera_contraria_secuencial();

	public:
		Equipo(gameMaster *belcebu, color equipo, 
				estrategia strat, int cant_jugadores, int quantum, vector<coordenadas> posiciones);
		void comenzar();
		void terminar();
		// crear jugadores

};
#endif // EQUIPO_H
