#ifndef juego_h
#define juego_h
#include <SFML/Graphics.hpp>

class Juego {
private:
    sf::RenderWindow ventana;
    Tamagotchi mascota;
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