#include "gtest-1.8.1/gtest.h"
#include <iostream>
#include <thread>
#include <sys/unistd.h>
#include <vector>
#include <mutex>
#include "../equipo.h"

// TESTS GLOBALES
TEST(EquipoTest, primero) {
    Config config = *(new Config());
    gameMaster belcebu = gameMaster(config);
	
    // Creo equipo
    const estrategia strat = SECUENCIAL;
	int quantum = 10;
	Equipo rojo(&belcebu, ROJO, strat, config.cantidad_jugadores, quantum, config.pos_rojo);
	
    EXPECT_TRUE(AZUL != ROJO);
}