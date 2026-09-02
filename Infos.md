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

//-----------------------------------------------------------//
/*
🚩1️⃣ Bouton Auto/Manuel
1. CSS ( lignes 231-240 )

  Bouton rectangulaire AUTO / MANUEL  
.mode-btn{
  min-width:92px; padding:8px 12px; border:none; cursor:pointer;
  border-radius:12px; /* 🚨bords arrondis -> ajuster ce rayon 
  font-size:.85rem; font-weight:900; letter-spacing:.04em; text-transform:uppercase; color:#fff;
  text-align:center; transition:background .2s, box-shadow .2s;
}
/* 🚨Couleur fond mode AUTO Orange*/
.mode-btn.auto{
  background:rgba(251, 195, 12, 1);
  border:5px solid rgba(134, 119, 79, 1);
}
/* 🚨Couleur fond mode MANUEL Vert */
.mode-btn.manuel{
  background:rgba(19, 151, 155, 1); 
  border: 5px solid rgba(17, 121, 125, 1);
  }

Ce code CSS sert à styliser un élément HTML qui possède à la fois la classe .mode-btn et la classe .manuel.
Voici l'explication détaillée des deux règles appliquées :
1. background: rgba(19, 151, 155, 1);
Cette ligne définit la couleur de fond de l'élément.rgba signifie Rouge, Vert, Bleu, et Alpha (l'opacité).
Les trois premiers chiffres (19, 151, 155) créent une couleur bleu-vert / turquoise.Le dernier chiffre 1 indique une opacité maximale (100%), 
ce qui signifie que la couleur est complètement opaque (pas de transparence).

2. border: 5px solid rgba(15, 65, 73, 0.8);
Cette ligne ajoute une bordure tout autour de l'élément.5px détermine l'épaisseur de la 
bordure (5 pixels).solid indique que la bordure est une ligne continue (pas de pointillés ni de tirets).rgba(15, 65, 73, 0.8) 
définit la couleur de cette bordure, qui est un bleu-vert très foncé. Le 0.8 signifie que la bordure a une opacité de 80%,
 elle laisse donc légèrement transparaître ce qui se trouve derrière elle.

pour les couleurs  https://rgbacolorpicker.com/

2. HTML (lignes 367-373, dans initRelays())

html
<div>
  <button type="button" class="mode-btn" id="${r.id}mode-btn" onclick="toggleMode('${r.id}')">---</button>
</div>

3. JavaScript ( lignes 400-401, dans update())

javascript
const modeBtn = document.getElementById(p + 'mode-btn');
modeBtn.innerText = d.auto ? "AUTO" : "MANUEL";
modeBtn.className = "mode-btn " + (d.auto ? "auto" : "manuel");

Rien d'autre n'a été touché : ni le back-end ESP32 (routes /toggle-mode, /get-data, etc.), ni le reste de l'interface.

.mode-btn.manuel{background:rgba(39, 245, 184, 1); box-shadow:0 0 12px rgba(39, 245, 184, 0.82);}
1. .mode-btn.manuel — le sélecteur CSS
Ça cible tous les éléments qui ont à la fois la classe mode-btn et la classe manuel (les deux classes collées sans espace = "ET", pas "OU"). 
C'est exactement ce que fait le JavaScript quand d.auto est false : modeBtn.className = "mode-btn manuel".

2. background:rgba(39, 245, 184, 1); — la couleur de fond du bouton
rgba(...) = un modèle de couleur à 4 valeurs
39 = quantité de Rouge (0 à 255)
245 = quantité de Vert (0 à 255)
184 = quantité de Bleu (0 à 255)
Ces trois valeurs donnent une teinte turquoise/vert d'eau (beaucoup de vert, pas mal de bleu, très peu de rouge)
1 = le canal Alpha (opacité), où 1 = 100% opaque (couleur pleine, on ne voit rien à travers)

3. box-shadow:0 0 12px rgba(39, 245, 184, 0.82); — l'ombre portée (le halo lumineux)
0 (1er) = décalage horizontal → 0 = pas de décalage sur les côtés
0 (2e) = décalage vertical → 0 = pas de décalage haut/bas
12px = le flou (blur radius) → plus le chiffre est grand, plus le halo est diffus/large
rgba(39, 245, 184, 0.82) = même couleur turquoise que le fond, mais avec 0.82 d'opacité (82%), donc légèrement transparente 
pour créer un effet de lueur qui se fond dans le fond sombre plutôt qu'un contour dur

//-------------------------------------------------
🚩2️⃣Code modifié pour le Switch Auto/Manuel

Ligne 369 — dans la construction HTML de chaque ligne de relais :
// Avant
<input type="checkbox" id="${r.id}auto-switch" onclick="fetch('/toggle-mode?id=${r.id}')">
// Après
<input type="checkbox" id="${r.id}auto-switch" onclick="toggleMode('${r.id}')">

Après la ligne 419 (juste après la fin de la fonction update(), avant saveRelay()) — ajout d'une nouvelle fonction :
async function toggleMode(id) {
  await fetch('/toggle-mode?id=' + id);
  update();
}*/

//-------------------------------------------------
🚩 3️⃣Ce que fait ce nouveau code :
Évite les faux démarrages : La boucle while bloque le programme pendant un court instant 
(généralement entre 1 et 3 secondes) jusqu'à ce que getLocalTime() renvoie une heure réelle.
Retour visuel : L'écran OLED affiche "Synchro heure Internet..." pour informer l'utilisateur 
de ce qui se passe.Sécurité (Timeout) : Si votre box internet est en panne ou que les serveurs 
NTP ne répondent pas, la boucle s'arrête d'elle-même après 10 secondes (tentative < 20) pour que l'ESP32 
démarre quand même son serveur web, vous permettant ainsi d'y accéder en local.

//-------------------------------------------------
// 🚩4️⃣  Commutation Réseau
1. Liste noire temporaire + reconnexion propre — lignes
 567–632 (fonction connectToBestNetwork())
 567-568 : nouvelles variables lastFailTime[] et BLACKLIST_DURATION
 577 : WiFi.disconnect(true) avant le scan
 598 : vérification blacklisted dans la sélection du meilleur réseau
fin de fonction (~ligne 620-630) : mise à jour de lastFailTime selon succès/échec

2. Retour automatique vers le meilleur réseau — lignes 
 637–685 (nouvelle fonction checkForBetterNetwork())
 645-646 : constantes BEST_NETWORK_RECHECK_INTERVAL (60 s) et RSSI_SWITCH_MARGIN (8 dB)
 648 : début de la fonction

3. Désactivation de la reconnexion auto interne — ligne 
831 (dans setup())
WiFi.setAutoReconnect(false);

4. Appel périodique dans la boucle principale — lignes 
 1064–1067 (dans loop())
appel de checkForBetterNetwork() toutes les 60 s

*/