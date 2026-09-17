# DOOM: Pathfinding — Operación Laberinto

Juego para Windows de tres sectores, desarrollado en C++17 y SFML 3.
Vista principal con raycasting y minimapa 2D sincronizado. Gráficos y sonidos
procedurales originales; no requiere archivos de DOOM.

## Jugar

Abre **dist/DoomPathfinding/juego.exe**, o extrae por completo
**DoomPathfinding-Windows.zip** y abre **juego.exe** dentro de la carpeta extraída.
Mantén las DLL junto al ejecutable. No hace falta instalar SFML para jugar.
Se usa Consolas o Arial de la instalación de Windows.

Pulsa Enter en el menú. Recoge la llave dorada, alcanza la salida verde y pulsa
Espacio. Completa los tres sectores para ganar. No es necesario eliminar a todos
los enemigos. La salud y munición se restablecen al entrar en cada sector.
Dos disparos eliminan un enemigo. Las cajas azules dan 18 balas y los botiquines
recuperan 40 puntos de salud. Los enemigos se activan al verte o escuchar disparos
cercanos, y usan BFS para rodear obstáculos. Sus ataques requieren proximidad y
línea de visión. Las rutas se recalculan durante la persecución.

| Tecla | Acción |
|---|---|
| W / S | Avanzar / retroceder |
| A / D | Desplazamiento lateral |
| Flecha izquierda / derecha | Girar |
| Ctrl | Disparar; se puede mantener pulsada |
| Shift izquierdo | Correr |
| Espacio | Usar salida cercana con llave |
| M | Mostrar / ocultar minimapa |
| P | Mostrar / ocultar rutas de enemigos |
| V | Activar / desactivar sonidos |
| Esc | Pausar / continuar; salir desde menú o pantalla final |
| Enter | Empezar / continuar / reintentar |
| R | Reiniciar sector desde pausa o derrota |

En el radar: blanco = jugador; rojo = enemigo; amarillo = llave; verde agua =
salida; azul = munición; blanco crema = botiquín. La pausa es automática cuando
la ventana pierde el foco. La ventana puede redimensionarse y conserva la proporción.

## Compilar

Requiere MSYS2 UCRT64 con GCC y SFML **3**. El proyecto usa las bibliotecas
instaladas en C:/msys64/ucrt64. Desde PowerShell en esta carpeta:

```powershell
.\build.ps1
# Otra instalación:
.\build.ps1 -Toolchain 'D:\msys64\ucrt64'
```

El script compila con advertencias, ejecuta las pruebas de lógica y crea una
carpeta portable y un ZIP con las DLL transitivas y avisos de licencia.
Para ejecutar las pruebas manualmente, con las DLL disponibles:
```powershell
.\dist\DoomPathfinding\juego.exe --self-test
```

Las pruebas validan conectividad de los tres mapas, acceso a llave/salida/enemigos,
rutas cardinales, límites, colisiones y rayos. No sustituyen probar la sensación
de movimiento o el equilibrio jugando.

## Organización

- src/Mapa.cpp: escenarios, colisiones, rayos DDA y rutas BFS.
- src/Personaje.cpp: movimiento y giro por teclado.
- src/Juego.cpp: campaña, combate, interfaz, renderizado y sonido.
- include/: interfaces de las clases.
- build.ps1: compilación, comprobaciones y distribución para Windows.

Alcance: campaña corta completa con estética retro; paredes de altura uniforme,
sin salto, desniveles, guardado de partida ni multijugador.

