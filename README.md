# Programmateur horaire ESP32 – multi-relais avec interface Web

Programme Arduino/ESP32 qui transforme une carte ESP32 en **programmateur horaire connecté**, capable de piloter plusieurs relais indépendants, configurable et supervisable depuis un simple navigateur web sur le réseau local, avec un écran OLED en bonus pour un suivi sans PC.

## ✨ Fonctionnalités

- **Nombre de relais configurable** : le nombre de sorties pilotées est défini par un simple tableau (`programmateurs[]`) dans le code. Ajouter ou retirer un relais ne demande aucune autre modification — la page web, les routes HTTP, la sauvegarde et l'écran OLED s'adaptent automatiquement.
- **Deux modes par relais** :
  - **Automatique** : le relais s'active/se désactive selon une plage horaire programmée (les plages traversant minuit, ex. 22:00–06:00, sont gérées).
  - **Manuel** : l'état est forcé ON/OFF par l'utilisateur depuis l'interface, indépendamment de l'horaire.
- **Interface web embarquée** : une seule page HTML/CSS/JS, servie directement depuis la mémoire flash de l'ESP32 (aucune carte SD ni système de fichiers requis). Elle interroge l'ESP32 en JSON pour afficher et modifier dynamiquement les relais, quel que soit leur nombre.
- **Sauvegarde persistante** : les horaires, modes et états sont stockés dans la mémoire flash NVS (bibliothèque `Preferences`) et survivent aux coupures de courant/redémarrages.
- **Wi-Fi multi-réseaux avec bascule intelligente** :
  - Se connecte automatiquement au réseau connu offrant le meilleur signal (RSSI) parmi une liste définie dans `arduino_secrets.h`.
  - Reconnexion automatique en cas de coupure, avec mise en liste noire temporaire d'un réseau qui vient d'échouer.
  - Vérifie périodiquement si un réseau habituellement meilleur est redevenu disponible et bascule dessus si l'écart de signal est significatif (hystérésis anti-va-et-vient).
- **Accès simplifié** : l'ESP32 est joignable via une adresse `http://<hostname>.local` grâce au mDNS, en plus de son adresse IP.
- **Horloge synchronisée** : l'heure est récupérée via NTP au démarrage, avec le fuseau horaire français (gestion automatique heure été/hiver).
- **Écran OLED (SSD1306, 0.96", I2C) optionnel** : affiche le réseau Wi-Fi utilisé, l'IP, l'heure, et pour chaque relais son mode (A/M), son état (ON/OFF) et sa plage horaire. Si le nombre de relais dépasse la place disponible, l'affichage bascule automatiquement en pages tournantes.

## 🧰 Matériel nécessaire

- Une carte **ESP32** (type DevKit).
- Un ou plusieurs **modules relais** (autant que de lignes déclarées dans `programmateurs[]`).
- Un **écran OLED SSD1306** 0.96" I2C (adresse `0x3C`) — optionnel, le programme fonctionne sans (il le détecte automatiquement au démarrage).
- Câblage I2C standard pour l'écran : SDA = GPIO 21, SCL = GPIO 22 (broches par défaut de l'ESP32).

### Broches GPIO conseillées pour les relais

| Utilisables | 4, 5, 13, 14, 16, 17, 18, 19, 21*, 22*, 23, 25, 26, 27, 32, 33 |
|---|---|
| **À éviter** | GPIO 34 à 39 (entrée seule), broches de boot 0, 2, 12, 15 |

\* GPIO 21/22 sont déjà utilisées par l'écran OLED (SDA/SCL) : ne pas les réutiliser pour un relais.

Le nombre maximal de relais dépend surtout du nombre de broches libres sur la carte (une quinzaine sur un ESP32 classique). Au-delà, il faudrait passer par un module d'extension I2C (ex. PCF8574), non inclus dans ce code.

## 📚 Bibliothèques Arduino requises

À installer via le gestionnaire de bibliothèques de l'IDE Arduino (ou PlatformIO) :

- `WiFi` (fournie avec le core ESP32)
- `ESPAsyncWebServer`
- `Preferences` (fournie avec le core ESP32)
- `ArduinoJson`
- `ESPmDNS` (fournie avec le core ESP32)
- `Wire` (fournie avec le core ESP32)
- `Adafruit GFX Library`
- `Adafruit SSD1306`

> `ESPAsyncWebServer` nécessite généralement aussi `AsyncTCP` (pour ESP32).

## ⚙️ Configuration avant le téléversement

### 1. Identifiants Wi-Fi (`arduino_secrets.h`)

Créez, à côté du fichier `.ino`, un fichier `arduino_secrets.h` (non fourni, à ne pas partager/committer publiquement) contenant au minimum :

```cpp
#define SECRET_SSID  "Nom_reseau_1"
#define SECRET_PASS  "mot_de_passe_1"
#define SECRET_SSID2 "Nom_reseau_2"
#define SECRET_PASS2 "mot_de_passe_2"

// Optionnel : décommenter pour un 3e réseau connu
// #define SECRET_SSID3 "Nom_reseau_3"
// #define SECRET_PASS3 "mot_de_passe_3"
```

L'ESP32 se connectera automatiquement au réseau de cette liste offrant le meilleur signal.

### 2. Nom d'hôte et fuseau horaire

Dans le `.ino`, adaptez si besoin :

```cpp
const char* hostname = "richardv";                             // accessible via http://richardv.local
const char *TZ_INFO  = "CET-1CEST,M3.5.0,M10.5.0/3";           // fuseau horaire (France par défaut)
```

### 3. Déclaration des relais (`programmateurs[]`)

C'est le cœur de la configuration. Chaque ligne = un relais :

```cpp
Programmateur programmateurs[] = {
  { "PR1", "Programmation 1", "Cuisine",  "#f59e0b", 32, "08:00", "18:00", true, false },
  { "PR2", "Programmation 2", "Portail",  "#06b6d4", 33, "09:00", "19:00", true, false },
  // ... ajoutez / supprimez des lignes selon vos besoins
};
```

| Champ | Description |
|---|---|
| `id` | Identifiant unique, court, sans espace (ex. `PR3`), utilisé dans les routes web et le stockage NVS |
| `nom` | Nom affiché sur la page web |
| `sousNom` | Sous-titre affiché (ex. la pièce ou l'usage) |
| `couleur` | Code couleur hexadécimal d'accent pour ce relais dans l'interface |
| `pin` | Broche GPIO reliée au module relais |
| `heureDebut` / `heureFin` | Horaires par défaut (au format `HH:MM`), écrasés par la NVS si déjà configurés depuis l'interface |
| `modeAuto` | `true` = démarre en mode automatique, `false` = démarre en mode manuel |
| `relayState` | État par défaut du relais (`false` = éteint) |

Aucune autre partie du code n'a besoin d'être modifiée pour changer le nombre de relais.

### 4. Écran OLED (facultatif)

Si aucun écran n'est branché ou détecté à l'adresse `0x3C`, le programme continue de fonctionner normalement (fonctionnalité web/relais intacte), il désactive simplement l'affichage.

## 🚀 Installation

1. Installez l'IDE Arduino (ou PlatformIO) et le support de cartes **ESP32**.
2. Installez les bibliothèques listées ci-dessus.
3. Créez le fichier `arduino_secrets.h` avec vos identifiants Wi-Fi (voir plus haut).
4. Adaptez le tableau `programmateurs[]` à votre câblage.
5. Sélectionnez votre carte ESP32 et le bon port série dans l'IDE.
6. Téléversez le programme.
7. Ouvrez le moniteur série (115200 bauds) pour suivre la connexion Wi-Fi et récupérer l'adresse IP attribuée.
8. Accédez à l'interface depuis un navigateur du même réseau local via :
   - `http://<adresse_ip_affichée>` ou
   - `http://<hostname>.local` (ex. `http://richardv.local`)

## 🖥️ Utilisation de l'interface web

Pour chaque relais, la page permet de :

- Voir l'état actuel (ON/OFF) et l'heure courante de l'ESP32.
- Basculer entre mode **Automatique** et mode **Manuel**.
- **Forcer** l'état ON/OFF (bascule automatiquement le relais en mode manuel).
- Modifier et enregistrer les **horaires de début/fin** de la plage automatique.
- Consulter les informations système (Wi-Fi connecté, IP, adresse MAC, puissance du signal) via un panneau d'info.

Toutes les actions sont envoyées en HTTP/JSON à l'ESP32, qui sauvegarde immédiatement les changements en mémoire flash (NVS).

## 🔌 API HTTP exposée

| Route | Méthode | Description |
|---|---|---|
| `/` | GET | Sert la page web principale |
| `/get-config` | GET | Liste des relais déclarés (id, nom, sous-titre, couleur) |
| `/get-data` | GET | État complet de tous les relais + heure courante (interrogée chaque seconde par la page) |
| `/get-info` | GET | Informations système : Wi-Fi, hostname, IP, MAC, RSSI |
| `/toggle-mode?id=XX` | GET | Bascule le relais `XX` entre mode auto et manuel |
| `/force-state?id=XX` | GET | Force le relais `XX` en mode manuel et inverse son état ON/OFF |
| `/save?id=XX` | POST (`debut`, `fin`) | Enregistre une nouvelle plage horaire pour le relais `XX` |

`XX` correspond à l'`id` défini dans `programmateurs[]` (ex. `PR1`).

## 🧠 Logique de fonctionnement

- La boucle principale (`loop()`) s'exécute une fois par seconde :
  1. Vérifie et rétablit la connexion Wi-Fi si besoin (avec bascule vers un meilleur réseau connu si disponible).
  2. Récupère l'heure courante (synchronisée NTP).
  3. Pour chaque relais : si en mode auto, calcule le nouvel état selon l'horaire (gère les plages traversant minuit) et ne modifie la broche physique que si l'état a changé ; si en mode manuel, réapplique l'état mémorisé.
  4. Met à jour l'écran OLED (avec rotation de pages toutes les 6 secondes si plusieurs pages sont nécessaires).

## ⚠️ Remarques et limites

- Le fichier `arduino_secrets.h` contenant vos identifiants Wi-Fi n'est pas fourni avec ce programme et ne doit pas être partagé publiquement (à exclure d'un dépôt Git public, par exemple via `.gitignore`).
- L'interface web n'intègre pas d'authentification : elle est prévue pour un usage sur réseau local de confiance.
- Le nombre de relais est limité par les broches GPIO disponibles sur la carte (adaptation possible via un module I2C d'extension, non inclus).
- En cas d'échec de connexion à tous les réseaux connus au démarrage, l'ESP32 redémarre automatiquement pour retenter.
