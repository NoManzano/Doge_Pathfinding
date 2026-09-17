#include "Mapa.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <queue>
Mapa::Mapa() { cargar(0); }
void Mapa::cargar(int nivel) {
    celdas = {
        "####################",
        "#..................#",
        "#..####...####.....#",
        "#.....#......#.....#",
        "#.....#......#.....#",
        "#............#.....#",
        "#..##...###........#",
        "#.......#..........#",
        "#.......#...####...#",
        "#..##..............#",
        "#...#...###........#",
        "#...#..............#",
        "#.......#...####...#",
        "#.......#..........#",
        "#..................#",
        "####################"
    };
    if (nivel == 1) {
        celdas[4][6]='.'; celdas[5][9]='#'; celdas[5][10]='#';
        celdas[9][12]='#'; celdas[9][13]='#'; celdas[12][14]='.';
    }
    if (nivel == 2) {
        celdas[2][4]='.'; celdas[6][8]='.'; celdas[8][14]='.';
        celdas[4][3]='#'; celdas[4][4]='#'; celdas[11][10]='#';
        celdas[11][11]='#'; celdas[13][8]='.';
    }
}
bool Mapa::esPared(int fila, int columna) const {
    return fila < 0 || columna < 0 || fila >= int(celdas.size()) ||
           columna >= int(celdas[0].size()) || celdas[fila][columna]=='#';
}
bool Mapa::libre(sf::Vector2f p, float r) const {
    for (int y=int(std::floor(p.y-r)); y<=int(std::floor(p.y+r)); ++y)
        for (int x=int(std::floor(p.x-r)); x<=int(std::floor(p.x+r)); ++x)
            if (esPared(y,x)) return false;
    return true;
}
std::vector<sf::Vector2i> Mapa::ruta(sf::Vector2f inicio, sf::Vector2f destino) const {
    const int w=int(celdas[0].size()), h=int(celdas.size());
    sf::Vector2i a{int(std::floor(inicio.x)),int(std::floor(inicio.y))};
    sf::Vector2i b{int(std::floor(destino.x)),int(std::floor(destino.y))};
    if (esPared(a.y,a.x)||esPared(b.y,b.x)) return {};

    auto indice=[w](int x,int y){return y*w+x;};
    auto heuristica=[b](int x,int y){
        return std::abs(x-b.x)+std::abs(y-b.y);
    };

    struct Nodo {
        int indice;
        int prioridad;
    };
    auto comparar=[](const Nodo& izquierdo,const Nodo& derecho){
        return izquierdo.prioridad>derecho.prioridad;
    };

    std::vector<int> padre(w*h,-1);
    std::vector<int> costo(w*h,std::numeric_limits<int>::max());
    std::priority_queue<Nodo,std::vector<Nodo>,decltype(comparar)> pendientes(comparar);
    int s=indice(a.x,a.y), fin=indice(b.x,b.y);
    padre[s]=s; costo[s]=0;
    pendientes.push({s,heuristica(a.x,a.y)});
    const sf::Vector2i direcciones[]={{1,0},{-1,0},{0,1},{0,-1}};
    while (!pendientes.empty()) {
        Nodo nodo=pendientes.top(); pendientes.pop();
        int actual=nodo.indice;
        int actualX=actual%w, actualY=actual/w;
        if (nodo.prioridad!=costo[actual]+heuristica(actualX,actualY)) continue;
        if (actual==fin) break;

        for (auto d:direcciones) {
            int x=actualX+d.x, y=actualY+d.y;
            if (esPared(y,x)) continue;
            int siguiente=indice(x,y);
            int nuevoCosto=costo[actual]+1;
            if (nuevoCosto>=costo[siguiente]) continue;
            costo[siguiente]=nuevoCosto;
            padre[siguiente]=actual;
            pendientes.push({siguiente,nuevoCosto+heuristica(x,y)});
        }
    }
    if (padre[fin]<0) return {};
    std::vector<sf::Vector2i> camino;
    for (int n=fin; n!=s; n=padre[n]) camino.push_back({n%w,n/w});
    std::reverse(camino.begin(),camino.end());
    return camino;
}
float Mapa::rayo(sf::Vector2f o, sf::Vector2f d, int* lado) const {
    int x=int(std::floor(o.x)),y=int(std::floor(o.y));
    float dx=std::abs(d.x)<.00001f?1e8f:std::abs(1.f/d.x);
    float dy=std::abs(d.y)<.00001f?1e8f:std::abs(1.f/d.y);
    int sx=d.x<0?-1:1, sy=d.y<0?-1:1;
    float tx=(d.x<0?o.x-x:x+1-o.x)*dx;
    float ty=(d.y<0?o.y-y:y+1-o.y)*dy;
    int l=0; float distancia=0;
    for (int i=0;i<128;++i) {
        if (tx<ty) {distancia=tx; tx+=dx; x+=sx; l=0;}
        else {distancia=ty; ty+=dy; y+=sy; l=1;}
        if(esPared(y,x)) break;
    }
    if(lado) *lado=l;
    return std::max(.001f,distancia);
}
bool Mapa::visible(sf::Vector2f a, sf::Vector2f b) const {
    auto d=b-a; float longitud=std::sqrt(d.x*d.x+d.y*d.y);
    return longitud<.001f || rayo(a,d/longitud)>longitud-.05f;
}

