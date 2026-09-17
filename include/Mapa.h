#pragma once
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>
class Mapa {
public:
    Mapa();
    void cargar(int nivel);
    const std::vector<std::string>& obtenerCeldas() const { return celdas; }
    bool esPared(int fila, int columna) const;
    bool libre(sf::Vector2f p, float radio = .2f) const;
    std::vector<sf::Vector2i> ruta(sf::Vector2f inicio, sf::Vector2f destino) const;
    float rayo(sf::Vector2f origen, sf::Vector2f direccion, int* lado = nullptr) const;
    bool visible(sf::Vector2f a, sf::Vector2f b) const;
private:
    std::vector<std::string> celdas;
};

