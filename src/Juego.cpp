#include "Juego.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <random>
#include <stdexcept>

namespace {
constexpr float W=1280,H=720, V=616, FOV=.66f;
const sf::Color cyan{84,225,220}, oro{255,194,88}, gris{146,164,177};
float longitud(sf::Vector2f p){return std::sqrt(p.x*p.x+p.y*p.y);}
sf::Color tono(sf::Color c,float t){
    return {std::uint8_t(std::clamp(c.r*t,0.f,255.f)),
            std::uint8_t(std::clamp(c.g*t,0.f,255.f)),
            std::uint8_t(std::clamp(c.b*t,0.f,255.f)),c.a};
}
}
Juego::Juego():ventana(sf::VideoMode({1280,720}),"DOOM: PATHFINDING | Operacion Laberinto"),profundidad(640) {
    ventana.setVerticalSyncEnabled(true);
    ventana.setKeyRepeatEnabled(false);
    const char* win=std::getenv("WINDIR");
    std::string fonts=std::string(win?win:"C:/Windows")+"/Fonts/";
    if(!fuente.openFromFile(fonts+"consola.ttf") && !fuente.openFromFile(fonts+"arial.ttf"))
        throw std::runtime_error("No se encontro una fuente de Windows.");
    iniciarAudio();
    cargarNivel(0);
    estado=Estado::Menu;
}
void Juego::iniciarAudio(){
    auto crear=[](sf::SoundBuffer& b,int tipo){
        std::vector<std::int16_t> s(7000);
        std::mt19937 rng(42); std::uniform_real_distribution<float> ruido(-1,1);
        for(size_t i=0;i<s.size();++i){
            float t=float(i)/22050,env=std::exp(-t*(tipo==0?18.f:12.f));
            float onda=tipo==0?ruido(rng)*.7f+std::sin(500*t)*.3f:
                std::sin(6.2831853f*(tipo==1?130.f:680.f)*t);
            s[i]=std::int16_t(onda*env*11000);
        }
        return b.loadFromSamples(s.data(),s.size(),1,22050,{sf::SoundChannel::Mono});
    };
    if(crear(disparoBuffer,0)) disparoAudio=std::make_unique<sf::Sound>(disparoBuffer);
    if(crear(impactoBuffer,1)) impactoAudio=std::make_unique<sf::Sound>(impactoBuffer);
    if(crear(objetoBuffer,2)) objetoAudio=std::make_unique<sf::Sound>(objetoBuffer);
}
void Juego::cargarNivel(int n){
    nivel=n; mapa.cargar(n); jugador=Personaje{}; llave=false;
    enemigos.clear(); objetos.clear();
    const sf::Vector2f posiciones[]={{9.5f,3.5f},{3.5f,7.5f},{10.5f,9.5f},
        {17.5f,6.5f},{6.5f,13.5f},{17.5f,13.5f},{11.5f,5.5f},{14.5f,11.5f}};
    for(int i=0;i<4+n*2;++i) {Enemigo e; e.p=posiciones[i]; enemigos.push_back(e);}
    objetos={{{17.5f,2.5f},0},{{7.5f,5.5f},1},{{15.5f,9.5f},1},
             {{2.5f,11.5f},2},{{10.5f,13.5f},2},{{12.5f,1.5f},1}};
    enfriamiento=fogonazo=dano=0; estado=Estado::Jugando;
    mensaje("SECTOR "+std::to_string(n+1)+" / 3  -  Busca la llave dorada");
}
void Juego::mensaje(const std::string& s){aviso=s;avisoTiempo=4.f;}
void Juego::ejecutar(){
    while(ventana.isOpen()){
        float dt=std::min(reloj.restart().asSeconds(),.05f);
        eventos();
        if(!ventana.isOpen()) break;
        if(estado==Estado::Jugando && ventana.hasFocus()) actualizar(dt);
        dibujar();
    }
}
void Juego::eventos(){
    using K=sf::Keyboard::Key;
    while(auto e=ventana.pollEvent()){
        if(e->is<sf::Event::Closed>()) ventana.close();
        if(e->is<sf::Event::FocusLost>() && estado==Estado::Jugando) estado=Estado::Pausa;
        if(const auto* k=e->getIf<sf::Event::KeyPressed>()){
            if(k->code==K::Escape){
                if(estado==Estado::Jugando) estado=Estado::Pausa;
                else if(estado==Estado::Pausa) estado=Estado::Jugando;
                else ventana.close();
            }
            if(k->code==K::Enter){
                if(estado==Estado::Menu || estado==Estado::Victoria){tiempo=0;bajas=0;cargarNivel(0);}
                else if(estado==Estado::Muerto) cargarNivel(nivel);
                else if(estado==Estado::Pausa) estado=Estado::Jugando;
            }
            if(k->code==K::R && (estado==Estado::Muerto || estado==Estado::Pausa)) cargarNivel(nivel);
            if(k->code==K::M) minimapa=!minimapa;
            if(k->code==K::P) rutas=!rutas;
            if(k->code==K::V) sonido=!sonido;
            if(k->code==K::Space && estado==Estado::Jugando) interactuar();
            if((k->code==K::LControl || k->code==K::RControl) && estado==Estado::Jugando) disparar();
        }
    }
}
void Juego::disparar(){
    if(enfriamiento>0) return;
    if(jugador.municion<=0){mensaje("Sin municion: recoge las cajas azules");enfriamiento=.6f;return;}
    --jugador.municion; enfriamiento=.28f; fogonazo=.10f;
    if(sonido && disparoAudio) disparoAudio->play();
    Enemigo* objetivo=nullptr; float cercano=20.f;
    sf::Vector2f dir{std::cos(jugador.angulo),std::sin(jugador.angulo)};
    for(auto& e:enemigos){
        if(e.vida<=0) continue;
        auto d=e.p-jugador.posicion; float dist=longitud(d);
        if(dist<8.f) e.alerta=true;
        float frontal=d.x*dir.x+d.y*dir.y;
        float lateral=std::abs(d.x*dir.y-d.y*dir.x);
        if(frontal>0 && lateral<.32f && dist<cercano && mapa.visible(jugador.posicion,e.p)){
            cercano=dist; objetivo=&e;
        }
    }
    if(objetivo){
        --objetivo->vida; objetivo->herido=.16f;
        if(objetivo->vida==0){++bajas; mensaje("Amenaza eliminada");}
    }
}
void Juego::interactuar(){
    if(longitud(jugador.posicion-salida)>1.35f){mensaje("Busca la salida verde en el minimapa");return;}
    if(!llave){mensaje("La salida necesita la llave dorada");return;}
    if(nivel==2) estado=Estado::Victoria;
    else cargarNivel(nivel+1);
}
void Juego::actualizar(float dt){
    tiempo+=dt; enfriamiento-=dt; fogonazo-=dt; dano-=dt; avisoTiempo-=dt;
    jugador.actualizar(dt,mapa);
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ||
       sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl)) disparar();
    for(auto& o:objetos){
        if(!o.activo || longitud(o.p-jugador.posicion)>.65f) continue;
        if(o.tipo==2 && jugador.vida==100) continue;
        o.activo=false;
        if(o.tipo==0){llave=true;mensaje("LLAVE OBTENIDA - Ve a la salida verde y pulsa ESPACIO");}
        if(o.tipo==1){jugador.municion+=18;mensaje("+18 municiones");}
        if(o.tipo==2){jugador.vida=std::min(100,jugador.vida+40);mensaje("+40 de salud");}
        if(sonido && objetoAudio) objetoAudio->play();
    }
    for(auto& e:enemigos){
        if(e.vida<=0) continue;
        e.ataque-=dt; e.recalculo-=dt; e.herido-=dt;
        float dist=longitud(e.p-jugador.posicion);
        if(dist<7.f && mapa.visible(e.p,jugador.posicion)) e.alerta=true;
        if(!e.alerta) continue;
        if(e.recalculo<=0){
            e.ruta=mapa.ruta(e.p,jugador.posicion);e.recalculo=.45f;
        }
        if(dist>.65f){
            sf::Vector2f destino=e.p;
            if(!e.ruta.empty()) destino={e.ruta[0].x+.5f,e.ruta[0].y+.5f};
            else if(mapa.visible(e.p,jugador.posicion)) destino=jugador.posicion;
            auto delta=destino-e.p; float len=longitud(delta);
            if(len>.02f){
                auto siguiente=e.p+delta/len*std::min(len,(.85f+nivel*.12f)*dt);
                bool ocupado=false;
                for(const auto& otro:enemigos)
                    if(&otro!=&e && otro.vida>0 && longitud(otro.p-siguiente)<.38f) ocupado=true;
                if(!ocupado){
                    if(mapa.libre({siguiente.x,e.p.y},.16f)) e.p.x=siguiente.x;
                    if(mapa.libre({e.p.x,siguiente.y},.16f)) e.p.y=siguiente.y;
                }
            }
            if(!e.ruta.empty() && longitud(destino-e.p)<.06f) e.ruta.erase(e.ruta.begin());
        }
        if(dist<.85f && e.ataque<=0 && mapa.visible(e.p,jugador.posicion)){
            jugador.vida=std::max(0,jugador.vida-(8+nivel*2));e.ataque=1.f;dano=.22f;
            if(sonido && impactoAudio) impactoAudio->play();
        }
    }
    if(jugador.vida<=0) estado=Estado::Muerto;
}
void Juego::rect(float x,float y,float w,float h,sf::Color c){
    if(w<=0||h<=0)return;
    sf::RectangleShape r({w,h});r.setPosition({x,y});r.setFillColor(c);ventana.draw(r);
}
void Juego::texto(const std::string& s,float x,float y,unsigned tam,sf::Color c){
    sf::Text t(fuente,s,tam);t.setPosition({x,y});t.setFillColor(c);ventana.draw(t);
}
void Juego::mundo(){
    const sf::Color acentos[]={{67,125,139},{137,99,66},{112,74,124}};
    sf::Color base=acentos[nivel];
    for(int y=0;y<int(V)/2;y+=4){
        float f=float(y)/(V/2);
        rect(0,float(y),W,4,sf::Color(std::uint8_t(10+f*12),std::uint8_t(15+f*16),std::uint8_t(25+f*20)));
        rect(0,V-y-4,W,4,sf::Color(std::uint8_t(19+f*6),std::uint8_t(26+f*7),std::uint8_t(32+f*8)));
    }
    sf::Vector2f dir{std::cos(jugador.angulo),std::sin(jugador.angulo)};
    sf::Vector2f plano{-dir.y*FOV,dir.x*FOV};
    for(int i=0;i<640;++i){
        float c=2.f*(i+.5f)/640-1;
        auto ray=dir+plano*c;int lado;
        float d=mapa.rayo(jugador.posicion,ray,&lado); profundidad[i]=d;
        float altura=V/d,top=(V-altura)/2;
        auto hit=jugador.posicion+ray*d;
        float u=lado?hit.x:hit.y;u-=std::floor(u);
        float luz=std::max(.18f,1.f/(1.f+d*.12f))*(lado?.74f:1.f);
        auto color=tono(base,luz);
        if(u<.025f || u>.975f) color=tono(color,.45f);
        rect(float(i*2),std::max(0.f,top),2,std::min(V,top+altura)-std::max(0.f,top),color);
        // Horizontal metal panels and a luminous strip, clipped to the viewport.
        for(float v:{.04f,.33f,.66f,.96f}){
            float yy=top+altura*v;
            if(yy>=0 && yy<V) rect(float(i*2),yy,2,std::min(V-yy,std::max(1.f,altura*.012f)),tono(color,.45f));
        }
        float yy=top+altura*.19f;
        if(yy>=0 && yy<V && u>.10f && u<.90f)
            rect(float(i*2),yy,2,std::min(V-yy,std::max(1.f,altura*.022f)),tono(cyan,std::max(.3f,luz)));
    }
    struct Sprite{sf::Vector2f p;int tipo;bool herido;};
    std::vector<Sprite> sprites;
    for(auto& e:enemigos) if(e.vida>0) sprites.push_back({e.p,3,e.herido>0});
    for(auto& o:objetos) if(o.activo) sprites.push_back({o.p,o.tipo,false});
    sprites.push_back({salida,4,false});
    std::sort(sprites.begin(),sprites.end(),[&](auto a,auto b){
        return longitud(a.p-jugador.posicion)>longitud(b.p-jugador.posicion);
    });
    // Original procedural pixel sprites; no external game assets.
    const std::vector<std::string> monster={
        "    RRRRRRRR    ","   RRRRRRRRRR   ","  RRYYYYYYRRRR  ","  RRYKYYKYRRRR  ",
        "  RRRRRRRRRRRR  ","   RRWWWWRRRR   ","    RRRRRRRR    ","  RRRRRRRRRRRR  ",
        " RRRRRRRRRRRRRR ","RRRRRROOOORRRRRR","RRR RRROORRR RRR","RRR RRRRRRRR RRR",
        "    RRRRRRRR    ","    RRR  RRR    ","   RRRR  RRRR   ","   KKKK  KKKK   "};
    for(auto s:sprites){
        auto delta=s.p-jugador.posicion;
        float z=delta.x*dir.x+delta.y*dir.y;
        if(z<.12f)continue;
        float lateral=delta.x*(-dir.y)+delta.y*dir.x;
        float centro=W/2*(1+lateral/(z*FOV));
        float escala=(s.tipo==3?.82f:s.tipo==4?1.f:.42f)*V/z;
        escala=std::min(escala,2400.f);
        float pie=V/2+V/(2*z),top=pie-escala;
        float ancho=escala*(s.tipo==4?.6f:.75f),izq=centro-ancho/2;
        int desde=std::max(0,int(izq)/2),hasta=std::min(639,int(izq+ancho)/2);
        for(int x=desde;x<=hasta;++x){
            if(z>profundidad[x]+.02f)continue;
            int px=std::clamp(int((x*2.f-izq)/ancho*16),0,15);
            for(int py=0;py<16;++py){
                sf::Color col; bool pinta=true;
                if(s.tipo==3){
                    char p=monster[py][px];
                    if(p==' ')continue;
                    col=p=='Y'?oro:p=='K'?sf::Color(19,14,25):p=='W'?sf::Color(240,221,182):
                        p=='O'?sf::Color(112,35,45):sf::Color(195,66,63);
                    if(s.herido)col=sf::Color::White;
                }else if(s.tipo==4){
                    pinta=px<2||px>13||py<2||py>13||((px==7||px==8)&&py>5);
                    col=llave?cyan:sf::Color(75,135,109);
                }else if(s.tipo==0){
                    pinta=(py>=3&&py<=8&&px>=3&&px<=10)||((px==7||px==8)&&py>=8)||(py==13&&px>=7&&px<=12);
                    col=oro;
                }else{
                    pinta=px>=2&&px<=13&&py>=5&&py<=14;
                    col=s.tipo==1?sf::Color(48,133,210):sf::Color(231,222,202);
                    if(s.tipo==2 && ((px>=7&&px<=8)||(py>=9&&py<=10)))col=sf::Color(216,62,76);
                    if(s.tipo==1 && px%3==0 && py>7 && py<13)col=oro;
                }
                float y=top+escala*py/16;
                float alto=escala/16+.5f;
                if(pinta && y<V && y+alto>0)
                    rect(x*2.f,std::max(0.f,y),2,std::min(V,y+alto)-std::max(0.f,y),tono(col,std::max(.38f,1.f/(1+z*.075f))));
            }
        }
    }
    // Weapon and crosshair.
    float rebote=fogonazo>0?16.f:0.f;
    rect(560,530+rebote,160,86,{34,43,53});
    rect(584,486+rebote,112,130,{60,74,85});
    rect(605,459+rebote,70,114,{103,117,123});
    rect(619,444+rebote,42,79,{31,38,48});
    rect(627,453+rebote,26,44,{12,19,26});
    rect(590,536+rebote,10,60,cyan);rect(680,536+rebote,10,60,cyan);
    if(fogonazo>0){
        rect(617,401,46,48,oro);rect(626,383,28,71,{255,239,182});rect(601,417,79,15,oro);
    }
    rect(631,V/2,6,2,{225,245,240});rect(643,V/2,6,2,{225,245,240});
    rect(639,V/2-9,2,6,{225,245,240});rect(639,V/2+5,2,6,{225,245,240});
    if(dano>0)rect(0,0,W,V,{210,30,40,75});
}
void Juego::dibujarMinimapa(){
    constexpr float x=990,y=58,s=12;
    rect(x-14,y-35,268,246,{9,17,27,240});
    texto("RADAR / SECTOR 0"+std::to_string(nivel+1),x,y-27,15,cyan);
    auto& c=mapa.obtenerCeldas();
    for(size_t f=0;f<c.size();++f)for(size_t col=0;col<c[f].size();++col)
        rect(x+col*s,y+f*s,s-1,s-1,c[f][col]=='#'?sf::Color(65,88,105):sf::Color(21,33,45));
    if(rutas)for(auto& e:enemigos)if(e.vida>0&&e.alerta)
        for(auto p:e.ruta)rect(x+p.x*s+4,y+p.y*s+4,3,3,{150,75,71});
    for(auto& o:objetos)if(o.activo)
        rect(x+o.p.x*s-3,y+o.p.y*s-3,6,6,o.tipo==0?oro:o.tipo==1?sf::Color(55,146,238):sf::Color(238,225,209));
    rect(x+salida.x*s-4,y+salida.y*s-4,8,8,cyan);
    for(auto& e:enemigos)if(e.vida>0)rect(x+e.p.x*s-3,y+e.p.y*s-3,6,6,{231,82,84});
    float px=x+jugador.posicion.x*s,py=y+jugador.posicion.y*s;
    sf::CircleShape p(4);p.setOrigin({4,4});p.setPosition({px,py});p.setFillColor(sf::Color::White);ventana.draw(p);
    for(int i=4;i<13;++i)rect(px+std::cos(jugador.angulo)*i,py+std::sin(jugador.angulo)*i,2,2,cyan);
    texto(rutas?"P: rutas ON   M: ocultar":"P: rutas OFF  M: ocultar",x,y+198,13,gris);
}
void Juego::dibujar(){
    ventana.clear({8,13,21});
    // Fixed logical canvas with letterboxing on any window size.
    auto size=ventana.getSize();
    sf::View view(sf::FloatRect({0,0},{W,H}));
    float ratio=size.y?float(size.x)/size.y:W/H;
    if(ratio>W/H){float a=(W/H)/ratio;view.setViewport({{(1-a)/2,0},{a,1}});}
    else{float a=ratio/(W/H);view.setViewport({{0,(1-a)/2},{1,a}});}
    ventana.setView(view);
    mundo();
    rect(0,0,W,38,{8,14,22,225});
    texto("DOOM: PATHFINDING",24,8,18,cyan);
    texto("OPERACION LABERINTO / 0"+std::to_string(nivel+1),410,9,16,gris);
    if(minimapa)dibujarMinimapa();
    rect(0,V,W,H-V,{9,16,25});rect(0,V,W,2,cyan);
    texto("SALUD",28,634,14,gris);texto(std::to_string(jugador.vida),28,654,32,jugador.vida<30?sf::Color(244,90,91):cyan);
    rect(132,666,150,12,{37,47,59});rect(132,666,jugador.vida*1.5f,12,cyan);
    texto("MUNICION",320,634,14,gris);texto(std::to_string(jugador.municion),320,654,32,oro);
    texto("ACCESO",490,634,14,gris);texto(llave?"LLAVE OK":"SIN LLAVE",490,666,18,llave?oro:gris);
    texto("BAJAS  "+std::to_string(bajas)+"     TIEMPO  "+std::to_string(int(tiempo))+"s",715,640,16,gris);
    texto("WASD mover | Flechas girar | CTRL disparar",715,665,15);
    texto("ESPACIO usar | SHIFT correr | ESC pausa",715,687,15,gris);
    if(avisoTiempo>0 && estado==Estado::Jugando){
        rect(22,558,850,39,{8,14,22,225});texto(aviso,36,568,17,oro);
    }
    if(estado==Estado::Jugando && longitud(jugador.posicion-salida)<1.35f){
        rect(348,365,574,44,{8,14,22,235});texto(llave?"ESPACIO  >  ACCEDER AL SIGUIENTE SECTOR":"SALIDA BLOQUEADA: NECESITAS LA LLAVE",367,378,18,oro);
    }
    if(estado!=Estado::Jugando){
        rect(0,0,W,H,{3,8,15,205});
        rect(145,94,990,512,{13,24,36,245});rect(145,94,5,512,cyan);
        texto("D O O M :  P A T H F I N D I N G",185,130,31,cyan);
        std::string titulo=estado==Estado::Menu?"OPERACION LABERINTO":estado==Estado::Pausa?"PARTIDA EN PAUSA":
            estado==Estado::Muerto?"SENAL PERDIDA":"MISION COMPLETADA";
        texto(titulo,185,193,34,estado==Estado::Muerto?sf::Color(242,101,103):oro);
        if(estado==Estado::Menu){
            texto("Tres sectores. Una salida. Encuentra la llave y sobrevive.",185,255,20);
            texto("W / S       Avanzar / retroceder",185,309,18,gris);
            texto("A / D       Desplazamiento lateral",185,340,18,gris);
            texto("FLECHAS     Girar izquierda / derecha",185,371,18,gris);
            texto("CTRL        Disparar (mantener)",185,402,18,gris);
            texto("ESPACIO     Usar la salida",185,433,18,gris);
            texto("SHIFT correr   M mapa   P rutas   V sonido",185,477,17,cyan);
        }else if(estado==Estado::Pausa){
            texto("ENTER / ESC: continuar     R: reiniciar sector",185,280,21);
            texto("WASD: mover  |  Flechas: girar  |  CTRL: disparar",185,336,18,gris);
            texto("ESPACIO: salida  |  SHIFT: correr  |  M: minimapa",185,374,18,gris);
            texto("P: rutas de enemigos  |  V: sonido",185,412,18,gris);
            texto(sonido?"SONIDO ACTIVADO":"SONIDO DESACTIVADO",185,471,17,cyan);
        }else if(estado==Estado::Muerto){
            texto("Los guardianes te alcanzaron en el sector "+std::to_string(nivel+1)+".",185,280,21);
            texto("Usa A / D para esquivar y recoge botiquines.",185,343,20,gris);
            texto("El radar muestra por donde vienen los enemigos.",185,382,20,gris);
            texto("ENTER / R: reintentar este sector",185,461,21,cyan);
        }else{
            texto("Has escapado de los tres sectores.",185,280,23);
            texto("Enemigos eliminados: "+std::to_string(bajas),185,345,22,gris);
            texto("Tiempo de expedicion: "+std::to_string(int(tiempo))+" segundos",185,386,22,gris);
            texto("ENTER: nueva expedicion",185,466,21,cyan);
        }
        texto(estado==Estado::Menu?"[ ENTER ]  INICIAR EXPEDICION":"[ ENTER ]  CONTINUAR",185,550,23,cyan);
        texto(estado==Estado::Pausa?"ESC: volver":"ESC: salir",928,557,15,gris);
    }
    ventana.display();
}
int Juego::pruebas(){
    int fallos=0,checks=0;
    auto comprobar=[&](bool b,const std::string& s){++checks;if(!b){++fallos;std::cerr<<"FALLO: "<<s<<"\n";}};
    for(int n=0;n<3;++n){
        Mapa m;m.cargar(n);
        comprobar(m.esPared(-1,0)&&m.esPared(100,100),"Limites seguros");
        comprobar(!m.libre({.9f,1.5f}),"Colision con borde");
        comprobar(std::abs(m.rayo({1.5f,1.5f},{-1,0})-.5f)<.001f,"Rayo horizontal");
        comprobar(std::abs(m.rayo({1.5f,1.5f},{0,-1})-.5f)<.001f,"Rayo vertical");
        comprobar(!m.visible({2.5f,2.5f},{7.5f,2.5f}),"Pared bloquea disparos");
        comprobar(m.visible({1.5f,1.5f},{10.5f,1.5f}),"Pasillo visible");
        for(int y=1;y<15;++y)for(int x=1;x<19;++x){
            if(m.esPared(y,x)||(x==1&&y==1))continue;
            auto ruta=m.ruta({1.5f,1.5f},{x+.5f,y+.5f});
            comprobar(!ruta.empty(),"Toda celda libre es alcanzable");
            sf::Vector2i prev{1,1};
            for(auto p:ruta){
                comprobar(!m.esPared(p.y,p.x)&&std::abs(p.x-prev.x)+std::abs(p.y-prev.y)==1,"Ruta cardinal sin paredes");
                prev=p;
            }
        }
        for(auto p:std::vector<sf::Vector2f>{{17.5f,2.5f},{18.5f,14.5f},{9.5f,3.5f},{3.5f,7.5f},{10.5f,9.5f},{17.5f,6.5f},{6.5f,13.5f},{17.5f,13.5f},{11.5f,5.5f},{14.5f,11.5f}})
            comprobar(m.libre(p)&&!m.ruta({1.5f,1.5f},p).empty(),"Objetivos y enemigos accesibles");
        Personaje p;p.mover({-30,0},m);
        comprobar(m.libre(p.posicion)&&p.posicion.x>=1.2f,"Sin atravesar paredes con pasos largos");
        comprobar(m.ruta({-1,-1},{1,1}).empty(),"Ruta invalida segura");
    }
    std::cout<<checks<<" comprobaciones, "<<fallos<<" fallos\n";
    return fallos?1:0;
}


int Juego::probarPartida(){
    int fallos=0;
    auto comprobar=[&](bool b,const std::string& s){
        std::cout<<(b?"OK: ":"FALLO: ")<<s<<"\n"; if(!b)++fallos;
    };
    sonido=false;
    std::filesystem::create_directories("qa");
    auto captura=[&](const std::string& nombre){
        dibujar(); // Populate both swap-chain buffers before reading back.
        dibujar();
        sf::Texture t;
        if(!t.resize(ventana.getSize())){++fallos;return;}
        t.update(ventana);
        if(!t.copyToImage().saveToFile("qa/"+nombre+".png"))++fallos;
    };
    estado=Estado::Menu;captura("menu");
    cargarNivel(0);
    jugador.posicion=salida;interactuar();
    comprobar(nivel==0 && !llave,"La salida requiere llave");
    jugador.posicion={17.5f,2.5f};actualizar(.01f);
    comprobar(llave,"Recoger llave");
    jugador.posicion=salida;interactuar();
    comprobar(nivel==1 && estado==Estado::Jugando && !llave,"Cambio de sector y reinicio de llave");
    cargarNivel(0);
    enemigos.clear();
    Enemigo e;e.p={4.5f,1.5f};enemigos.push_back(e);
    disparar();comprobar(enemigos[0].vida==1 && jugador.municion==35,"Impacto frontal y consumo de municion");
    disparar();comprobar(enemigos[0].vida==1 && jugador.municion==35,"Cadencia limita disparos");
    enfriamiento=0;disparar();
    comprobar(enemigos[0].vida==0 && bajas==1,"Dos disparos eliminan enemigo");
    enfriamiento=0;jugador.municion=0;disparar();
    comprobar(jugador.municion==0,"Municion no se vuelve negativa");
    cargarNivel(0);enemigos.clear();
    jugador.posicion={2.5f,2.5f};e=Enemigo{};e.p={7.5f,2.5f};enemigos.push_back(e);
    disparar();comprobar(enemigos[0].vida==2,"Disparo no atraviesa pared");
    cargarNivel(0);enemigos.clear();
    jugador.vida=20;jugador.posicion={2.5f,11.5f};actualizar(.01f);
    comprobar(jugador.vida==60,"Botiquin recupera salud");
    jugador.posicion={7.5f,5.5f};actualizar(.01f);
    comprobar(jugador.municion==54,"Caja repone municion");
    cargarNivel(0);enemigos.clear();
    e=Enemigo{};e.p={2.f,1.5f};e.alerta=true;enemigos.push_back(e);
    jugador.vida=1;actualizar(.01f);
    comprobar(estado==Estado::Muerto && jugador.vida==0,"Derrota por contacto enemigo");
    cargarNivel(0);
    comprobar(estado==Estado::Jugando && jugador.vida==100 && jugador.municion==36,"Reinicio recupera estado jugable");
    // Simulate pursuit around obstacles, independently from live keyboard input.
    enemigos.clear();e=Enemigo{};e.p={9.5f,3.5f};e.alerta=true;enemigos.push_back(e);
    bool colision=false;
    for(int i=0;i<2200;++i){
        jugador.vida=100;actualizar(.02f);
        if(!mapa.libre(enemigos[0].p,.15f))colision=true;
    }
    comprobar(!colision,"Enemigo no cruza paredes durante persecucion");
    comprobar(longitud(enemigos[0].p-jugador.posicion)<.9f,"Enemigo alcanza al jugador rodeando obstaculos");
    cargarNivel(0);jugador.posicion={8.5f,3.5f};jugador.angulo=0;
    captura("partida");
    estado=Estado::Pausa;captura("pausa");
    estado=Estado::Muerto;captura("derrota");
    // Follow the actual shortest path to key and exit in each map using movement.
    for(int n=0;n<3;++n){
        cargarNivel(n);enemigos.clear();
        auto caminar=[&](sf::Vector2f destino){
            auto camino=mapa.ruta(jugador.posicion,destino);
            for(auto cell:camino){
                sf::Vector2f meta{cell.x+.5f,cell.y+.5f};
                for(int i=0;i<30 && longitud(meta-jugador.posicion)>.02f;++i){
                    auto d=meta-jugador.posicion;
                    jugador.mover(d/std::max(.001f,longitud(d))*std::min(.08f,longitud(d)),mapa);
                    actualizar(.01f);
                }
            }
            return longitud(destino-jugador.posicion)<.1f;
        };
        comprobar(caminar({17.5f,2.5f}) && llave,"Ruta jugable hasta llave del sector "+std::to_string(n+1));
        comprobar(caminar(salida),"Ruta jugable hasta salida del sector "+std::to_string(n+1));
        interactuar();
        comprobar(n==2?estado==Estado::Victoria:nivel==n+1,"Progresion del sector "+std::to_string(n+1));
    }
    captura("victoria");
    std::cout<<"Pruebas de partida: "<<fallos<<" fallos\n";
    ventana.close();
    return fallos?1:0;
}

