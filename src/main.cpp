#include "Juego.h"
#include <iostream>
#include <string>
int main(int argc,char** argv) {
    try {
        if(argc>1 && std::string(argv[1])=="--self-test") return Juego::pruebas();
        Juego juego;
        if(argc>1 && std::string(argv[1])=="--smoke-test") return juego.probarPartida();
        juego.ejecutar();
    } catch(const std::exception& e) {std::cerr<<e.what()<<"\n";return 1;}
    return 0;
}
