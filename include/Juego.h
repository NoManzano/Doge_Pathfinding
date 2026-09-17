#ifndef juego_h
#define juego_h
#include <SFML/Graphics.hpp>
#include "Mapa.h"
#include "Personaje.h"

class Juego {
private:
    Personaje personaje;
    Mapa mapa;
    sf::RenderWindow ventana;
    sf::Clock reloj;
    float tiempoTranscurrido;

    void procesarEventos();
    void actualizar();
    void dibujar();

public:
    Juego();
    void ejecutar();
};

#endif // juego_h