# Programmateur Horaire ESP32 — Multi-Relais avec Interface Web

Ce projet transforme un ESP32 en programmateur horaire connecté, capable de piloter un **nombre configurable de relais** (4 par défaut, extensible) depuis une interface web moderne accessible sur votre réseau local.

## ✨ Fonctionnalités

- **Nombre de relais configurable** : ajoutez ou retirez des relais en modifiant un simple tableau dans le code, sans toucher au reste du programme.
- **Deux modes par relais** :
  - **Automatique** : le relais suit une plage horaire programmée (ex: 08:00–18:00), y compris les plages traversant minuit (ex: 22:00–06:00).
  - **Manuel** : forçage ON/OFF direct par l'utilisateur.
- **Interface web responsive** (mobile/desktop), servie directement par l'ESP32 (aucun fichier externe, aucune carte SD requise).
- **Actions groupées** : Tout ON / Tout OFF / Tout AUTO en un clic.
- **Écran OLED** (SSD1306 0.96", I2C) affichant le réseau WiFi, l'adresse IP, et l'état de chaque relais. Bascule automatiquement en pages si plus de 5 relais sont configurés.
- **Reconnexion WiFi automatique** avec sélection du meilleur réseau parmi plusieurs réseaux connus (par signal RSSI).
- **Accès simplifié via mDNS** (`http://richardv.local`).
- **Sauvegarde persistante** (mémoire flash NVS) : les réglages survivent aux coupures de courant et aux redémarrages.

## 🧰 Matériel nécessaire

- Une carte ESP32 (DevKit ou équivalent)
- 1 à 15 modules relais (selon le nombre de broches GPIO disponibles)
- Un écran OLED I2C SSD1306 0.96" (128×64, adresse `0x3C`) — optionnel mais recommandé
- Câblage I2C standard : `SDA = GPIO21`, `SCL = GPIO22`

## 📦 Bibliothèques Arduino requises

À installer via le gestionnaire de bibliothèques de l'IDE Arduino :

| Bibliothèque | Usage |
|---|---|
| `WiFi` | Connexion réseau (incluse avec le core ESP32) |
| `ESPAsyncWebServer` | Serveur web asynchrone |
| `Preferences` | Stockage des réglages en NVS (incluse avec le core ESP32) |
| `ArduinoJson` | Sérialisation JSON des routes API |
| `ESPmDNS` | Résolution de nom local `.local` (incluse avec le core ESP32) |
| `Wire` | Bus I2C (incluse avec le core ESP32) |
| `Adafruit_GFX` | Bibliothèque graphique de base |
| `Adafruit_SSD1306` | Pilote de l'écran OLED |

> `ESPAsyncWebServer` nécessite aussi sa dépendance `AsyncTCP` (installée automatiquement selon la source utilisée).

## ⚙️ Configuration avant compilation

### 1. Identifiants WiFi — `arduino_secrets.h`

Créez un fichier `arduino_secrets.h` dans le même dossier que le `.ino`, avec ce contenu :

```cpp
#define SECRET_SSID  "NomDeVotreReseau1"
#define SECRET_PASS  "MotDePasse1"
#define SECRET_SSID2 "NomDeVotreReseau2"
#define SECRET_PASS2 "MotDePasse2"

// Optionnel : décommentez pour ajouter un 3e réseau connu
// #define SECRET_SSID3 "NomDeVotreReseau3"
// #define SECRET_PASS3 "MotDePasse3"
```

⚠️ Ne partagez jamais ce fichier publiquement (ajoutez-le à votre `.gitignore` si vous utilisez Git).

### 2. Nombre et configuration des relais — dans le `.ino`

Toute la configuration des relais se fait dans **un seul tableau**, en haut du fichier :

```cpp
Programmateur programmateurs[] = {
  { "PR1", "Programmation 1", "Cuisine",  "#f59e0b", 32, "08:00", "18:00", true, false },
  { "PR2", "Programmation 2", "Portail",  "#06b6d4", 33, "09:00", "19:00", true, false },
  { "PR3", "Programmation 3", "Relais 3", "#0CE892", 25, "10:00", "20:00", true, false },
  { "PR4", "Programmation 4", "Relais 4", "#ef4444", 26, "11:00", "21:00", true, false },
};
```

| Champ | Description |
|---|---|
| `id` | Identifiant unique, court, sans espace (ex: `"PR5"`) |
| `nom` | Nom affiché dans l'interface (ex: `"Programmation 5"`) |
| `sousNom` | Sous-titre affiché (ex: `"Chauffage"`) |
| `couleur` | Couleur d'accent en hexadécimal (ex: `"#a78bfa"`) |
| `pin` | Broche GPIO reliée au relais |
| `heureDebut` / `heureFin` | Horaires par défaut au premier démarrage (format `"HH:MM"`) |
| `modeAuto` | `true` = mode automatique par défaut |
| `relayState` | État par défaut (`false` = éteint) |

**Pour ajouter un relais** : dupliquez une ligne, changez au minimum `id` et `pin`.
**Pour en retirer un** : supprimez la ligne correspondante.

Aucune autre partie du code n'a besoin d'être modifiée : la page web, les routes HTTP, la sauvegarde NVS et l'écran OLED s'adaptent automatiquement au nombre de lignes du tableau.

#### ⚠️ Broches GPIO — points d'attention

- **Utilisables en sortie** (ESP32 DevKit classique) : `2, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33`
- **Déjà utilisées** par l'écran OLED : `21` (SDA) et `22` (SCL) — ne pas les réutiliser pour un relais
- **À éviter** : GPIO 34 à 39 (entrée seule, pas de sortie possible)
- **Prudence** avec les broches de boot (`0, 2, 12, 15`) : un relais y étant branché peut perturber le démarrage si son état électrique n'est pas neutre au boot
- **Au-delà d'une quinzaine de relais** : les broches GPIO libres d'un ESP32 classique sont épuisées ; il faut alors passer par un module d'extension I2C (ex: `PCF8574`) — non inclus dans ce projet

### 3. Fuseau horaire

Par défaut réglé sur la France (heure d'été/hiver automatique) :

```cpp
const char *TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0/3";
```

Pour un autre pays, remplacez cette chaîne par la valeur POSIX TZ correspondante (recherchable en ligne : "POSIX TZ string [pays]").

### 4. Nom d'hôte mDNS

```cpp
const char* hostname = "richardv";
```

Modifiez cette valeur pour changer l'adresse d'accès (`http://<hostname>.local`).

## 🚀 Installation

1. Copiez `arduino_secrets.h` (voir ci-dessus) dans le dossier du sketch.
2. Adaptez le tableau `programmateurs[]` à votre montage.
3. Installez les bibliothèques listées plus haut via l'IDE Arduino (ou PlatformIO).
4. Sélectionnez votre carte ESP32 et le bon port dans l'IDE.
5. Téléversez le sketch.
6. Ouvrez le moniteur série (115200 bauds) pour vérifier la connexion WiFi et récupérer l'adresse IP.
7. Accédez à l'interface via `http://<adresse-ip>` ou `http://richardv.local` (ou le nom d'hôte que vous avez choisi).

## 🖥️ Utilisation de l'interface web

- **Cartes de relais** : chacune affiche l'état actuel (ON/OFF), un interrupteur AUTO/MANUEL, et selon le mode :
  - en **AUTO** : les champs "Début"/"Fin" à éditer puis valider avec **✓**
  - en **MANUEL** : un bouton **⚡ Forcer ON / OFF**
- **Boutons rapides** en haut de page : Tout ON / Tout OFF / Tout AUTO, appliqués à tous les relais en une seule action.
- **Icône 📶** (en haut à droite) : ouvre une fenêtre d'informations système (WiFi, IP, adresse MAC, signal).

## 🔌 API HTTP (routes du serveur)

| Route | Méthode | Description |
|---|---|---|
| `/` | GET | Sert la page web principale |
| `/get-config` | GET | Liste des relais configurés (id, nom, sous-titre, couleur) |
| `/get-data` | GET | État complet de tous les relais + heure courante |
| `/get-info` | GET | Informations système (WiFi, IP, MAC, RSSI) |
| `/toggle-mode?id=PRx` | GET | Bascule le relais `PRx` entre AUTO et MANUEL |
| `/force-state?id=PRx` | GET | Force l'inversion d'état du relais `PRx` (passe en MANUEL) |
| `/save?id=PRx` | POST | Enregistre les horaires `debut`/`fin` du relais `PRx` |

Ces routes sont génériques (paramétrées par `id`) et fonctionnent quel que soit le nombre de relais déclarés.

## 💾 Sauvegarde des réglages

Les réglages (horaires, mode, état) sont stockés dans la mémoire flash NVS de l'ESP32 via la bibliothèque `Preferences`, dans le namespace `"config"`. Ils sont automatiquement rechargés à chaque démarrage et survivent aux coupures de courant.

## 🐛 Dépannage

| Problème | Piste |
|---|---|
| Écran OLED éteint / message "Ecran OLED non detecte" au démarrage | Vérifiez le câblage I2C (SDA=21, SCL=22) et l'adresse I2C (`0x3C` par défaut) |
| L'ESP32 redémarre en boucle au démarrage | Aucun réseau WiFi connu n'a été détecté ; vérifiez `arduino_secrets.h` et la portée du signal |
| Page web inaccessible via `.local` | Le mDNS n'est pas toujours supporté par tous les réseaux/appareils ; utilisez directement l'adresse IP affichée dans le moniteur série |
| Un relais ne répond pas | Vérifiez que sa broche GPIO n'entre pas en conflit avec une autre utilisation (I2C, boot) |

## 📄 Licence

Projet personnel — libre d'utilisation et de modification.
