#pragma once
#include "Personaje.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <memory>
class Juego {
public:
    Juego();
    void ejecutar();
    static int pruebas();
    int probarPartida();
private:
    enum class Estado {Menu,Jugando,Pausa,Muerto,Victoria};
    struct Enemigo {
        sf::Vector2f p;
        int vida=2;
        float ataque=0, recalculo=0, herido=0;
        bool alerta=false;
        std::vector<sf::Vector2i> ruta;
    };
    struct Objeto {sf::Vector2f p; int tipo; bool activo=true;};
    Mapa mapa;
    Personaje jugador;
    sf::RenderWindow ventana;
    sf::Font fuente;
    sf::Clock reloj;
    Estado estado=Estado::Menu;
    std::vector<Enemigo> enemigos;
    std::vector<Objeto> objetos;
    sf::Vector2f salida{18.5f,14.5f};
    int nivel=0,bajas=0;
    bool llave=false, rutas=true, minimapa=true, sonido=true;
    float tiempo=0, enfriamiento=0, fogonazo=0, dano=0, avisoTiempo=0;
    std::string aviso;
    std::vector<float> profundidad;
    sf::SoundBuffer disparoBuffer, impactoBuffer, objetoBuffer;
    std::unique_ptr<sf::Sound> disparoAudio, impactoAudio, objetoAudio;
    void cargarNivel(int n);
    void eventos();
    void actualizar(float dt);
    void disparar();
    void interactuar();
    void dibujar();
    void mundo();
    void dibujarMinimapa();
    void rect(float x,float y,float w,float h,sf::Color color);
    void texto(const std::string& s,float x,float y,unsigned tam,sf::Color color=sf::Color::White);
    void mensaje(const std::string& s);
    void iniciarAudio();
};

