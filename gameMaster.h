#ifndef GAMEMASTER_H
#define GAMEMASTER_H
#include <tuple>
#include <cstdio>
#include <vector>
#include <mutex>
#include <semaphore.h>
#include "definiciones.h"
#include "config.h"
#include <atomic>

using namespace std;

class gameMaster {
private:
    // Atributos Privados
    int nro_ronda = 0;
    int x, y, jugadores_por_equipos; 
    vector<vector<color>> tablero; 
    vector<coordenadas> pos_jugadores_azules, pos_jugadores_rojos; 
    coordenadas pos_bandera_roja, pos_bandera_azul; 
    color turno; 
    estrategia stratActual;
	
    // Agregamos mas atributos
    int quantum; // si es -1, la estrategia no cuenta con quantum. Podria ser un puntero al quantum del equipo
    int quantumsOriginales[2];
    estrategia strats[2];
    int moviendose = 0; // el jugador esta en proceso de moverse

    // Métodos privados
    color obtener_coordenadas(coordenadas coord);
    void mover_jugador_tablero(coordenadas pos_anterior, coordenadas pos_nueva, color colorEquipo);
    
    // Agregamos mas metodos
    
 
public:
    // Atributos públicos
    gameMaster(Config config);
    void termino_ronda(color equipo); // Marca que un jugador terminó la ronda
    int mover_jugador(direccion dir, int nro_jugador);
    color ganador = INDEFINIDO;
    atomic_int semRojo = 0; // Variable para DEBUGGING
    atomic_int semAzul = 0; // Variable para DEBUGGING
    atomic_int dormidos[2] = {0,0}; // Cuantos estan dormidos por equipo

    // Agregamos mas atributos
    mutex mtx;
    // Métodos públicos
    bool termino_juego();
	int getTamx();
	int getTamy();
    static int distancia(coordenadas pair1, coordenadas pair2);
    sem_t turno_rojo, turno_azul, termina_ronda_rojo, termina_ronda_azul; // FIXME: Al principio necesito entrar como azul, luego puedo hacerlo por el método termino_ronda....
    color en_posicion(coordenadas coord);
    bool es_posicion_valida(coordenadas pos);
    bool es_color_libre(color color_tablero);
	coordenadas proxima_posicion(coordenadas anterior, direccion movimiento); // Calcula la proxima posición a moverse	
    
    // Agregamos mas metodos
    void setearEstrategia(estrategia strategy, color equipo);
    void setearQuantum(int q, color equipo);
    color getTurno();
    coordenadas pos_contraria(color equipo);
};

#endif // GAMEMASTER_H
