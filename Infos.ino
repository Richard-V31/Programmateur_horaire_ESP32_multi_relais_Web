/*
Pour rendre ça flexible, il faut refactoriser vers une architecture basée sur un tableau de programmateurs plutôt que des variables séparées PR1/PR2/PR3/PR4 :

Une structure Programmateur { pin, heureDebut, heureFin, modeAuto, relayState, nom }
Un tableau Programmateur programmateurs[N] où N est défini par la configuration
Des routes web génériques (ex: /toggle-mode?id=2) au lieu d'une route par relais
La page HTML JS génère déjà dynamiquement ses lignes depuis un tableau RELAYS — donc côté affichage c'est presque prêt, il « suffit » d'adapter le nombre d'entrées
La sauvegarde NVS en boucle sur le tableau au lieu de champs nommés PR1/PR2/PR3/PR4

  { "PR1", "Programmation 1", "Cuisine",  "#f59e0b", 32, "08:00", "18:00", true, false },
//  👉🚩Champs : { id, nom affiché, sous-titre, couleur (hex), broche GPIO,
//              heure de début par défaut, heure de fin par défaut,
//              mode auto par défaut, état par défaut }

Un tableau de configuration unique, avec des routes web génériques. Ajouter un relais deviendra une simple ligne à copier-coller dans le tableau, 
sans toucher au reste du code.

Un seul endroit à modifier : le tableau programmateurs[] . Chaque ligne = un relais (id, nom, couleur, broche GPIO, horaires par défaut). 
Pour passer de 4 à 8 relais, il suffit d'ajouter 4 lignes — rien d'autre à toucher.
 Une seule route gère n'importe quel nombre de relais.
Page web adaptative : le JavaScript interroge une nouvelle route /get-config au chargement pour savoir combien de relais existent 
et comment les afficher — la mise en page n'est plus figée dans le HTML.
Sauvegarde NVS en boucle : les clés sont générées dynamiquement (PR1debut, PR2debut...) au lieu d'être écrites une par une.
Écran OLED en pages tournantes : comme il ne peut afficher que 5 relais à la fois, l'écran bascule automatiquement toutes les 4 secondes 
vers la page suivante si vous dépassez 5 relais.

🚨 Pour aller au-delà de 4 relais :
L'ESP32 classique offre une quinzaine de broches GPIO utilisables en sortie (j'ai mis la liste en commentaire dans le code) — au-delà, il faudrait un 
module d'extension I2C (PCF8574).

  { "PR1", "Programmation 1", "Cuisine",  "#f59e0b", 32, "08:00", "18:00", true, false },
#f59e0b est un code couleur hexadécimal, le même format que ceux utilisés en CSS/HTML pour définir une couleur.

Décomposition :

# indique que ce qui suit est un code couleur hexadécimal
06 → composante rouge (0 à 255, ici 6 en décimal = très faible)
b6 → composante verte (182 en décimal = assez forte)
d4 → composante bleue (212 en décimal = forte)

Chaque paire de caractères est un nombre en base 16 (hexadécimal), de 00 (aucune intensité) à ff (intensité maximale).
f59e0b donne donc un mélange peu de rouge + beaucoup de vert + beaucoup de bleu, ce qui produit un cyan/turquoise :
Composantes RVB de #f59e0b
Intensité (0-255)
Rouge
Vert
Bleu

Dans le code, c'est simplement la couleur d'accent attribuée au relais "PR2 / Portail" : elle sert à colorer le liseré à gauche de sa carte dans l'interface 
web (border-left:3px solid var(--accent)), ainsi que certains effets visuels au survol/appui du bouton "Forcer" (via --accent-rgb, qui est calculé 
automatiquement en JS avec la fonction hex2rgb()).

On peut remplacer cette valeur par n'importe quel code hexadécimal pour changer la couleur associée à ce relais — par exemple 
#ec4899 pour du rose, ou 
#22c55e pour du vert.

//-------------------------------------------------
🚩1️⃣Code modifié pour le Switch Auto/Manuel

Ligne 369 — dans la construction HTML de chaque ligne de relais :
// Avant
<input type="checkbox" id="${r.id}auto-switch" onclick="fetch('/toggle-mode?id=${r.id}')">
// Après
<input type="checkbox" id="${r.id}auto-switch" onclick="toggleMode('${r.id}')">

Après la ligne 419 (juste après la fin de la fonction update(), avant saveRelay()) — ajout d'une nouvelle fonction :
async function toggleMode(id) {
  await fetch('/toggle-mode?id=' + id);
  update();
}

//-----------------------------------------------------------//
🚩2️⃣Cette ligne définit l'apparence du petit rond blanc (le curseur) du switch — le .track est le fond ovale, et ::before crée un élément virtuel positionné par-dessus, sans avoir besoin d'ajouter une vraie balise HTML.
.toggle .track::before{content:""; position:absolute; height:17px; width:17px; left:4px; top:1px; background:#fff; border-radius:50%; transition:transform .2s; box-shadow:0 1px 3px rgba(0,0,0,.4);}

Détail propriété par propriété :
content:"" — obligatoire pour qu'un pseudo-élément ::before s'affiche ; ici on ne veut pas de texte, juste une forme, donc vide.
position:absolute — positionne le rond par rapport à .track (qui doit avoir position:relative ailleurs dans le CSS), et non par rapport à la page.
height:17px; width:17px — taille du rond, 17×17 pixels.
left:4px; top:1px — place le rond à 4px du bord gauche et 1px du haut de la piste : c'est sa position quand le switch est décoché (position "off").
background:#fff — couleur blanche du rond.
border-radius:50% — transforme le carré 17×17 en cercle parfait.
transition:transform .2s — anime en douceur (0,2 seconde) tout changement de la propriété transform. C'est ce qui donne l'effet de glissement fluide du curseur quand il passe de gauche à droite.
box-shadow:0 1px 3px rgba(0,0,0,.4) — une légère ombre portée sous le rond (décalée de 1px vers le bas, flou de 3px, noir à 40% d'opacité), pour lui donner un effet de relief.

Le mouvement du rond de gauche à droite (quand on coche la case) est en général géré par une autre règle du type .toggle input:checked + .track::before { transform: translateX(...) }, 
qui déplace le rond via transform, animé grâce à la transition définie ici.

//-------------------------------------------
Switch Auto/Manuel
Le switch est un <input type="checkbox"> classique. 
Piste : 38×21 px → 56×30 px
Bouton rond : 15 px → 22 px
Décalage quand activé : ajusté en conséquence (26 px au lieu de 17 px) pour que le bouton reste bien centré des deux côtés

Élément	                      Avant	    Après  Position
Largeur piste	                 38px	    56px    1️⃣
Hauteur piste	                 21px	    30px    1️⃣
Diamètre du rond	             15px	    18px    2️⃣
Position initiale (left/top)	  3px	    4px     3️⃣
Déplacement quand activé	     17px	   26px    4️⃣

.toggle{position:relative; width:56px; height:30px; <--1️⃣ flex-shrink:0; et translateX(26px)} 
.toggle input{opacity:0; width:0; height:0;}
.toggle .track{position:absolute; inset:0; cursor:pointer; background:rgba(255,255,255,.14); border-radius:999px; transition:background .2s;}
.toggle .track::before{content:""; position:absolute; height:18px; width:18px; <-- 2️⃣ left:4px; top:4px;<-- 3️⃣ background:#fff; border-radius:50%; transition:transform .2s; box-shadow:0 1px 3px rgba(0,0,0,.4);}
.toggle input:checked + .track{background:var(--accent);}
.toggle input:checked + .track::before{transform:translateX(26px);} <--4️⃣
*/
