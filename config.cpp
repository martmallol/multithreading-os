#include "config.h"


Config::Config() {
    ifstream config("./config/config_parameters.csv");
    if(!config){
        cerr << "Error: el archivo no pudo ser abierto" << endl;
        exit(1);
    }
    int bandera_roja_first, bandera_roja_second, bandera_azul_first, bandera_azul_second;
    
    config >> x 
           >> y 
           >> cantidad_jugadores
           >> bandera_roja_first 
           >> bandera_roja_second 
           >> bandera_azul_first 
           >> bandera_azul_second;

    bandera_roja = make_pair(bandera_roja_first, bandera_roja_second);    
    bandera_azul = make_pair(bandera_azul_first, bandera_azul_second);
    int posiciones_iniciales_first, posiciones_iniciales_second;
    for(int i = 0; i < cantidad_jugadores; i++){
        config >> posiciones_iniciales_first >> posiciones_iniciales_second;
        pos_rojo.push_back(make_pair(posiciones_iniciales_first, posiciones_iniciales_second));
    }

    for(int i = 0; i < cantidad_jugadores; i++){
        config >> posiciones_iniciales_first >> posiciones_iniciales_second;
        pos_azul.push_back(make_pair(posiciones_iniciales_first, posiciones_iniciales_second));
    }

    
};

