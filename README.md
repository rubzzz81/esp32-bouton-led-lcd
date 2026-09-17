# ESP32 - Bouton, LED et écran Grove LCD RGB

Projet PlatformIO pour ESP32-WROOM-32 (carte `rymcu-esp32-devkitc`, un clone du
DevKitC générique Espressif). Un bouton poussoir fait défiler un statut de
présence ("Libre", "En consultation", "Absent reviens plus tard", "Absent
toute la journée") affiché sur un écran Grove LCD RGB Backlight, avec une LED
qui reflète l'état "occupé / libre".

## Matériel

- ESP32-WROOM-32 DevKitC (ou clone compatible, ex. RYMCU)
- 1 bouton poussoir
- 1 LED + résistance série (ou la LED intégrée de la carte sur GPIO2)
- 1 écran [Grove - LCD RGB Backlight](https://wiki.seeedstudio.com/Grove-LCD_RGB_Backlight/) (I2C)

## Câblage

| Composant        | Broche ESP32       | Remarque                                              |
|-------------------|--------------------|--------------------------------------------------------|
| LED (anode)        | GPIO2 (via résistance) | Cathode à GND. Certaines cartes ont déjà une LED sur GPIO2. |
| Bouton              | GPIO4              | Autre patte du bouton à GND. Pull-up interne activée (`INPUT_PULLUP`), donc appui = niveau LOW. |
| Écran Grove LCD RGB - SDA | GPIO21       | Bus I2C par défaut de l'ESP32 sous Arduino.            |
| Écran Grove LCD RGB - SCL | GPIO22       |                                                          |
| Écran Grove LCD RGB - VCC | 5V ou 3.3V selon le module | Vérifier la tension supportée par le module. |
| Écran Grove LCD RGB - GND | GND          |                                                          |

## Historique du projet / choix techniques

### 1. Diagnostic initial : le code ne compilait pas

Le projet contenait un fichier `led.ino` (code Arduino classique avec
`setup()`/`loop()`) à la racine, mais [platformio.ini](platformio.ini) était
configuré avec `framework = espidf` et le vrai point d'entrée du build était
`src/main.c` (`void app_main() {}`, vide). Résultat :

- `led.ino` n'était même pas compilé (PlatformIO ne compile que ce qui est
  dans `src/`).
- Même déplacé dans `src/`, du code Arduino ne peut pas compiler sous
  ESP-IDF pur : `setup()`, `loop()`, `pinMode()`, `digitalWrite()` n'existent
  pas dans cette API.

**Correction :**
- `framework` passé de `espidf` à `arduino` dans `platformio.ini`.
- Code déplacé et adapté dans `src/main.cpp` (avec `#include <Arduino.h>`).
- Suppression du stub `src/main.c` (ESP-IDF) et de `led.ino`, devenus inutiles.
- Vérification que la carte `rymcu-esp32-devkitc` (vendeur RYMCU) est bien
  identique en caractéristiques (mcu `esp32`, flash 4 Mo, etc.) au module
  générique `esp32dev` d'Espressif — donc bien compatible avec un ESP32-WROOM-32.

### 2. Ajout de l'écran Grove LCD RGB Backlight

Ajout de la librairie `seeed-studio/Grove - LCD RGB Backlight` dans
`lib_deps` de `platformio.ini`. Cet écran communique en I2C (bus `Wire`), sur
les broches par défaut de l'ESP32 (SDA = GPIO21, SCL = GPIO22) — pas besoin de
préciser les broches, `Wire.begin()` sans arguments suffit sur ESP32/Arduino.

### 3. Affichage du statut du bouton (version 1 : ON/OFF)

Première version : l'écran affichait simplement "LED : ALLUMEE" / "LED :
ETEINTE" avec un fond vert/rouge, mis à jour uniquement quand l'état du
bouton changeait (évite de rafraîchir l'écran à chaque tour de `loop()`).

### 4. Cycle de statuts à chaque appui (version actuelle)

Le bouton ne suit plus l'état "maintenu" mais fonctionne par **appuis
successifs** : chaque appui fait avancer un index de statut (0 à 3, avec
retour à 0 après le dernier). Détection sur le **front montant** (transition
relâché → appuyé) pour ne déclencher qu'une seule fois par appui, avec un
`delay(50)` anti-rebond basique après le changement.

Statuts, répartis sur les 2 lignes de l'écran (16 caractères max par ligne) :

| # | Ligne 1            | Ligne 2       | Couleur fond      |
|---|---------------------|----------------|--------------------|
| 0 | `Libre, frappe`      | `et entre !`   | Vert               |
| 1 | `En`                 | `consultation` | Orange             |
| 2 | `Absent reviens`     | `plus tard`    | Orange foncé       |
| 3 | `Absent toute`       | `la journee`   | Rouge              |

**Remarques :**
- Les phrases trop longues pour 16 colonnes sont coupées à un mot entier sur
  deux lignes.
- Le mot "journée" est volontairement écrit sans accent ("journee") : les
  écrans LCD à caractères (contrôleur HD44780) n'affichent pas correctement
  les caractères accentués encodés en UTF-8 (ils sortent comme des symboles
  incorrects).
- La LED (GPIO2) s'allume dès que le statut n'est pas "Libre" (index 0), et
  s'éteint sur "Libre". La couleur du rétroéclairage RGB de l'écran change
  aussi avec chaque statut, pour un repérage visuel rapide.

## Structure du projet

```
platformio.ini     Configuration PlatformIO (carte, framework, librairies)
src/main.cpp        Code principal (bouton, LED, écran LCD)
```

## Compilation et upload

```
pio run              # compiler
pio run -t upload    # flasher sur la carte
pio device monitor    # moniteur série (si besoin de logs)
```
