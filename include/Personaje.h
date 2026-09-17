#pragma once
#include "Mapa.h"
class Personaje {
public:
    sf::Vector2f posicion{1.5f,1.5f};
    float angulo=0.f;
    int vida=100, municion=36;
    void actualizar(float delta, const Mapa& mapa);
    void mover(sf::Vector2f desplazamiento, const Mapa& mapa);
};

