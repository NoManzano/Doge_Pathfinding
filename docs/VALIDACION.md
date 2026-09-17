# Validación de la versión jugable

- Compilación: GCC/UCRT64, C++17, SFML 3; -Wall -Wextra, sin advertencias.
- Pruebas de mapas: 10 508 comprobaciones, 0 fallos.
- Pruebas integradas (--smoke-test): 23 escenarios, 0 fallos.
- Se verificó: llave obligatoria, tres transiciones hasta victoria, rutas caminables,
  impactos, cadencia, munición, paredes que bloquean disparos, botiquines,
  daño, derrota, reinicio y persecución alrededor de obstáculos.
- Se abrió la distribución portable en Windows y se revisaron visualmente
  la vista principal, el minimapa y la pausa.
- Las pruebas de progresión recorren los niveles sin enemigos para aislar los
  objetivos. El combate y la persecución se comprueban en escenarios separados.
  No equivalen a una campaña completa jugada manualmente ni a una prueba de
  dificultad con varios jugadores.

Comandos:
```powershell
.\build.ps1
.\dist\DoomPathfinding\juego.exe --self-test
.\dist\DoomPathfinding\juego.exe --smoke-test
```

La última opción abre una ventana de pruebas, guarda capturas en qa y termina
con código 0 si todo pasa. No se incluye en el menú de juego.

