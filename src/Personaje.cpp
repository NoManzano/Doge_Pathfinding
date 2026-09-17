#include "Personaje.h"

Personaje::Personaje()
    : posicion({80.0f, 80.0f}) {
}

void Personaje::dibujar(sf::RenderWindow& ventana) const {
    sf::CircleShape jugador(16.0f);
    jugador.setFillColor(sf::Color::Green);
    jugador.setPosition(posicion);

    ventana.draw(jugador);
}

void Personaje::actualizar(float delta) {
    const float velocidad = 200.0f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
        posicion.y -= velocidad * delta;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
        posicion.y += velocidad * delta;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
        posicion.x -= velocidad * delta;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
        posicion.x += velocidad * delta;
    }
}