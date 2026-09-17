#include "Personaje.h"
#include <SFML/Window/Keyboard.hpp>
#include <algorithm>
#include <cmath>
void Personaje::mover(sf::Vector2f d, const Mapa& mapa) {
    // Substeps prevent tunnelling even if a frame stalls.
    int pasos=std::max(1,int(std::ceil(std::sqrt(d.x*d.x+d.y*d.y)/.1f)));
    d/=float(pasos);
    for(int i=0;i<pasos;++i) {
        if(mapa.libre({posicion.x+d.x,posicion.y})) posicion.x+=d.x;
        if(mapa.libre({posicion.x,posicion.y+d.y})) posicion.y+=d.y;
    }
}
void Personaje::actualizar(float dt,const Mapa& mapa) {
    using K=sf::Keyboard::Key;
    auto pulsada=[](K k){return sf::Keyboard::isKeyPressed(k);};
    angulo+=(float(pulsada(K::Right))-float(pulsada(K::Left)))*2.2f*dt;
    angulo=std::remainder(angulo,6.2831853f);
    float frente=float(pulsada(K::W))-float(pulsada(K::S));
    float lateral=float(pulsada(K::D))-float(pulsada(K::A));
    float norma=std::max(1.f,std::sqrt(frente*frente+lateral*lateral));
    float velocidad=pulsada(K::LShift)?3.5f:2.4f;
    mover({(std::cos(angulo)*frente-std::sin(angulo)*lateral)*velocidad*dt/norma,
           (std::sin(angulo)*frente+std::cos(angulo)*lateral)*velocidad*dt/norma},mapa);
}

