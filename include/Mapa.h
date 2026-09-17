#ifndef MAPA_H
#define MAPA_H

#include <string>
#include <vector>

class Mapa {
private:
    std::vector<std::string> celdas;

public:
    Mapa();

    const std::vector<std::string>& obtenerCeldas() const;
    bool esPared(int fila, int columna) const;
};

#endif