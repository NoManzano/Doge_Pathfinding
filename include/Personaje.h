#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <SFML/Graphics.hpp>

class Personaje {
private:
    sf::Vector2f posicion;

public:
    Personaje();

    void dibujar(sf::RenderWindow& ventana) const;
};

#endif