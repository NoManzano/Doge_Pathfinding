#include "Mapa.h"

Mapa::Mapa() {
    celdas = {
        "########",
        "#......#",
        "#..##..#",
        "#......#",
        "########"
    };
}

const std::vector<std::string>& Mapa::obtenerCeldas() const {
    return celdas;
}

bool Mapa::esPared(int fila, int columna) const {
    return celdas[fila][columna] == '#';
}