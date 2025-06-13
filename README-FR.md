# PaxOS 9

![Logo PaxOS 9](https://github.com/paxo-phone/PaxOS-9/assets/45568523/ddb3b517-605c-41b4-8c1e-c8e5d156431b)

[![PlatformIO CI](https://github.com/paxo-phone/PaxOS-9/actions/workflows/platformio-ci.yml/badge.svg)](https://github.com/paxo-phone/PaxOS-9/actions/workflows/platformio-ci.yml)
[![License: AGPL v3](https://img.shields.io/badge/License-AGPL_v3-blue.svg)](https://www.gnu.org/licenses/agpl-3.0)
[![Discord](https://img.shields.io/discord/747816102250643506?label=Discord&logo=discord)](https://discord.com/invite/MpqbWr3pUG)

**PaxOS 9** est la dernière itération de PaxOS, un système d'exploitation léger et open-source conçu principalement pour les appareils PaxoPhone. Il est construit à l'aide de l'écosystème PlatformIO, permettant le développement et le déploiement sur son matériel cible ainsi que des simulations natives pour les environnements de bureau.

## Table des Matières

- [Qu'est-ce que PaxOS 9 ?](#quest-ce-que-paxos-9-)
- [Fonctionnalités Clés](#fonctionnalités-clés)
- [Matériel Cible](#matériel-cible)
- [Pour Commencer](#pour-commencer)
  - [Prérequis](#prérequis)
  - [Cloner le Dépôt](#cloner-le-dépôt)
- [Compiler et Exécuter PaxOS 9](#compiler-et-exécuter-paxos-9)
  - [1. Pour le Matériel PaxoPhone (ESP32 - `paxo-v5`)](#1-pour-le-matériel-paxophone-esp32---paxo-v5)
    - [Compilation (Build)](#compilation-build)
    - [Téléversement (Upload)](#téléversement-upload)
    - [Moniteur Série](#moniteur-série)
  - [2. Pour Windows (Simulation Native)](#2-pour-windows-simulation-native)
    - [Compiler & Exécuter](#compiler--exécuter)
    - [Compiler Seulement](#compiler-seulement)
  - [3. Pour Linux (Simulation Native)](#3-pour-linux-simulation-native)
    - [Compilation (Build)](#compilation-build-1)
    - [Exécuter](#exécuter)
  - [4. Pour macOS (Simulation Native)](#4-pour-macos-simulation-native)
    - [Compilation (Build)](#compilation-build-2)
    - [Exécuter](#exécuter-1)
- [Exécuter les Tests](#exécuter-les-tests)
- [Contribuer](#contribuer)
- [Ressources](#ressources)
- [Licence](#licence)
- [Contributeurs](#contributeurs)

## Qu'est-ce que PaxOS 9 ?

PaxOS 9 est un système d'exploitation adapté au PaxoPhone, un appareil basé sur ESP32. Il vise à fournir une expérience utilisateur fonctionnelle et réactive sur du matériel aux ressources limitées. L'OS s'appuie sur le framework Arduino pour sa cible ESP32 et inclut des bibliothèques pour les graphismes (LovyanGFX), l'entrée tactile (FT6236G), et plus encore. Pour le développement et les tests, PaxOS 9 peut également être compilé et exécuté comme une application native sur Windows, Linux et macOS, en utilisant SDL2 pour la simulation graphique.

## Fonctionnalités Clés

*   **Léger :** Conçu pour fonctionner efficacement sur les microcontrôleurs ESP32.
*   **Open Source :** Distribué sous la licence AGPL-3.0, encourageant les contributions de la communauté.
*   **Basé sur PlatformIO :** Simplifie le processus de compilation et la gestion des dépendances sur différentes plateformes.
*   **Abstraction Matérielle :** Prend en charge des composants matériels spécifiques tels que les écrans et les écrans tactiles.
*   **Simulation Native :** Permet aux développeurs de compiler, exécuter et tester sur des environnements de bureau (Windows, Linux, macOS) en utilisant SDL2 pour la simulation de l'interface utilisateur.

## Matériel Cible

La cible matérielle principale pour PaxOS 9 est le **PaxoPhone V5**, qui est basé sur la carte **ESP-WROVER-KIT** (un module ESP32 avec PSRAM).

## Pour Commencer

Suivez ces étapes pour rendre PaxOS 9 opérationnel sur votre machine ou appareil cible.

### Prérequis

1.  **Git :** Pour cloner le dépôt.
2.  **PlatformIO Core CLI :** PaxOS 9 utilise PlatformIO pour compiler et gérer le projet. Suivez le guide d'installation officiel : [https://docs.platformio.org/page/core/installation.html](https://docs.platformio.org/page/core/installation.html)
3.  **Pour les Compilations Natives sur Bureau (Windows, Linux, macOS) :**
    *   Un compilateur C++ (GCC, Clang, ou MSVC selon votre OS).
    *   **Bibliothèques de développement SDL2 :** Nécessaires pour la simulation graphique.
        *   **Linux (Debian/Ubuntu) :** `sudo apt-get install libsdl2-dev`
        *   **macOS (avec Homebrew) :** `brew install sdl2`
        *   **Windows :** Le projet inclut des scripts pour aider à gérer les dépendances SDL2 (voir la section de compilation Windows). Vous devrez peut-être vous assurer que MinGW (si vous utilisez GCC) ou les outils de compilation MSVC sont correctement configurés.
    *   **Python :** Requis pour certains scripts de pré-compilation, en particulier pour l'environnement Windows.
4.  **Pour les Compilations Natives sur Linux :**
    *   **Bibliothèques de développement NCurses :** `sudo apt-get install libncurses-dev` (ou équivalent pour votre distribution).
    *   **Autres Bibliothèques :** Vous pourriez avoir à installer d'autres bibliothèques en fonction de votre système.

### Cloner le Dépôt

```bash
git clone https://github.com/paxo-phone/PaxOS-9.git
cd PaxOS-9
```

## Compiler et Exécuter PaxOS 9

PaxOS 9 utilise les environnements PlatformIO pour gérer différentes configurations de compilation. Voici les commandes courantes :

*   Pour compiler un environnement spécifique : `pio run -e <nom_environnement>`
*   Pour compiler et téléverser (pour les cibles matérielles) : `pio run -e <nom_environnement> -t upload`
*   Pour nettoyer les fichiers de compilation d'un environnement : `pio run -e <nom_environnement> -t clean`

### 1. Pour le Matériel PaxoPhone (ESP32 - `paxo-v5`)

Cet environnement cible la carte ESP-WROVER-KIT, qui est la base du PaxoPhone V5.

#### Compilation (Build)

```bash
pio run -e paxo-v5
```

#### Téléversement (Upload)

Connectez votre PaxoPhone/ESP-WROVER-KIT à votre ordinateur via USB. PlatformIO tentera de détecter automatiquement le port.

```bash
pio run -e paxo-v5 -t upload
```
Si vous rencontrez des problèmes avec la détection du port, vous pouvez le spécifier en utilisant l'option `upload_port` dans `platformio.ini` ou via la ligne de commande.

#### Moniteur Série

Pour afficher la sortie série de l'appareil (par exemple, pour le débogage) :

```bash
pio device monitor -e paxo-v5 -b 115200
```
Le débit en bauds est réglé sur `115200`.

### 2. Pour Windows (Simulation Native)

Cela vous permet d'exécuter PaxOS 9 comme une application de bureau sur Windows pour le développement et les tests, en utilisant SDL2 pour l'affichage.

#### Compiler & Exécuter

L'environnement `windows` inclut des scripts pour configurer l'espace de travail, copier les dépendances (comme SDL2), et exécuter le programme compilé.

```bash
pio run -e windows
```
Cette commande compilera le projet puis tentera d'exécuter l'exécutable résultant. Les DLL SDL2 nécessaires devraient être copiées dans le répertoire de compilation par le script `copy_dependencies.py`.

#### Compiler Seulement

Si vous souhaitez uniquement compiler le projet sans l'exécuter immédiatement :

```bash
pio run -e windows-build-only
```
L'exécutable se trouvera généralement dans `.pio/build/windows/program.exe`.

### 3. Pour Linux (Simulation Native)

Cela vous permet d'exécuter PaxOS 9 comme une application de bureau sur Linux, en utilisant SDL2.

#### Compilation (Build)

Assurez-vous d'avoir installé les bibliothèques de développement SDL2 et NCurses (voir [Prérequis](#prérequis)).

```bash
pio run -e linux
```

#### Exécuter

Après une compilation réussie, vous pouvez exécuter l'exécutable :

```bash
.pio/build/linux/program
```

### 4. Pour macOS (Simulation Native)

Cela vous permet d'exécuter PaxOS 9 comme une application de bureau sur macOS, en utilisant SDL2.

#### Compilation (Build)

Assurez-vous d'avoir installé SDL2 en utilisant Homebrew (voir [Prérequis](#prérequis)). Les indicateurs de compilation sont configurés pour rechercher SDL2 dans les chemins d'installation courants de Homebrew (`/usr/local/include/SDL2`, `/opt/homebrew/include/SDL2`).

```bash
pio run -e macos
```

#### Exécuter

Après une compilation réussie, vous pouvez exécuter l'exécutable :

```bash
.pio/build/macos/program
```

### Pour toutes les compilations, si vous les exécutez vous-même, vous devez les lancer depuis le même répertoire que celui contenant `./storage/` (qui contient les fichiers de la carte SD).

## Exécuter les Tests

PaxOS 9 utilise GoogleTest pour ses tests unitaires. Ces tests sont généralement exécutés sur une plateforme native.

Pour compiler et exécuter les tests :

```bash
pio test -e test
```
Cette commande compilera le code avec la configuration de l'environnement `test` et exécutera les tests définis. L'environnement `test` est configuré pour une exécution native et peut ignorer certaines bibliothèques non pertinentes pour les tests unitaires (par exemple, les graphismes).

## Contribuer

Les contributions sont les bienvenues ! Si vous souhaitez contribuer à PaxOS 9, veuillez suivre ces étapes générales :

1.  Forker (créer une copie personnelle) le dépôt.
2.  Créer une nouvelle branche pour votre fonctionnalité ou correction de bug : `git checkout -b feature/votre-nom-de-fonctionnalite` ou `git checkout -b fix/votre-correction-de-bug`.
3.  Effectuez vos modifications et commitez-les avec des messages clairs et descriptifs.
4.  Assurez-vous que votre code se compile avec succès pour les environnements pertinents (en particulier `paxo-v5` et les environnements natifs si applicable).
5.  Si vous ajoutez de nouvelles fonctionnalités, envisagez d'ajouter des tests.
6.  Poussez votre branche vers votre fork : `git push origin feature/votre-nom-de-fonctionnalite`.
7.  Ouvrez une Pull Request (demande de fusion) vers la branche `main` du dépôt `paxo-phone/PaxOS-9`.

Veuillez vérifier s'il existe un fichier `CONTRIBUTING.md` pour des directives plus spécifiques.

## Ressources

*   **Site Web Officiel :** [paxo.fr](https://www.paxo.fr)
*   **Serveur Discord :** Rejoignez notre communauté sur [Discord](https://discord.com/invite/MpqbWr3pUG) pour des discussions, de l'aide et des mises à jour.

## Licence

Ce projet est distribué sous la **Licence Publique Générale Affero GNU v3.0 (AGPL-3.0)**. Voir le fichier `LICENSE` pour plus de détails.

## Contributeurs

Merci à toutes les personnes incroyables qui ont contribué à PaxOS 9 !

<a href="https://github.com/paxo-phone/PaxOS-9/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=paxo-phone/PaxOS-9" />
</a>
