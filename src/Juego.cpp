#include "Juego.h"

Juego::Juego()
    : mapa(),
      personaje(),
      ventana(sf::VideoMode({1408, 768}), "DOOM: Pathfinding"),
      tiempoTranscurrido(0.0f) {
}

void Juego::ejecutar() {
    while (ventana.isOpen()) {
        procesarEventos();

        if (!ventana.isOpen())
            break;

        actualizar();
        dibujar();
    }
}

void Juego::procesarEventos() {
    while (const auto evento = ventana.pollEvent()) {
        
        if (evento->is<sf::Event::Closed>()) {
            ventana.close();
        }
    }
}

void Juego::actualizar() {
    tiempoTranscurrido = reloj.restart().asSeconds();
    personaje.actualizar(tiempoTranscurrido);
}

void Juego::dibujar() {
    ventana.clear(sf::Color(230, 230, 230));

    const auto& celdas = mapa.obtenerCeldas();
    constexpr float tamanoCelda = 64.0f;

    for (std::size_t fila = 0; fila < celdas.size(); ++fila) {
        for (std::size_t columna = 0; columna < celdas[fila].size(); ++columna) {
            sf::RectangleShape rectangulo;
            rectangulo.setSize({tamanoCelda, tamanoCelda});
            rectangulo.setPosition({
                static_cast<float>(columna) * tamanoCelda,
                static_cast<float>(fila) * tamanoCelda
            });

            if (celdas[fila][columna] == '#') {
                rectangulo.setFillColor(sf::Color(40, 40, 40));
            } else {
                rectangulo.setFillColor(sf::Color(180, 180, 180));
            }

            ventana.draw(rectangulo);
        }
    }

    personaje.dibujar(ventana);

    ventana.display();
}