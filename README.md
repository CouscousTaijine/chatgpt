# Mini FPS 3D ultra-léger (C)

Prototype FPS 3D très minimal en **un seul fichier C** (`mini_fps.c`) avec bots simples:

- déplacement FPS (WASD + souris)
- tir hitscan (clic gauche)
- bots cubes qui poursuivent le joueur
- respawn automatique des bots
- score + timer de partie

## Dépendance

Ce prototype utilise **raylib** (lib graphique très légère).

## Build (Linux/macOS)

```bash
gcc mini_fps.c -o mini_fps -O2 -lraylib -lm -ldl -lpthread -lX11
```

> Selon votre plateforme, les flags peuvent varier.

## Lancer

```bash
./mini_fps
```

## Contrôles

- `WASD` : se déplacer
- `Souris` : regarder
- `Clic gauche` : tirer
- `ESC` : quitter
