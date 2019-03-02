 ==============================================================================
| Jeu Where2Go v2 (labyrinthe avec murs invisibles et des méchants)	       |
| Version 1.0 02/03/2019	      					       |
| Auteur : BOTTE Florian et VARIN Alexandre (L3 Info Passerelle session 2018)  |
 ==============================================================================

*** Note d'utilisation ***
----------------------------------------------------------------------------
1. Compilé tout le dossier : 
	- Compilé le dossier contrôleur avec la magnifique commande "make"

2. Lancer le contrôleur :
	- lancer le contrôleur avec la commande "./controleur CCC XXX YYY ZZZ" où :
		- CCC est le nom de la carte à charger en mémoire
		- XXX est la clé de la file de message
		- YYY est la clé du segment de mémoire partagé
		- ZZZ est la clé du tableau de sémaphore

3. Lancer le joueur (pour jouer) :
	- lancer le joueur (max 1 joueur) avec la commande "./joueur XXX" où :
		- XXX est la clé de la file de message crée par le contrôleur

4. Lancer des minotaures (pour rajouter du piment) :
	- lancer des minotaures (max 5 minotaures) avec la commande "./minotaure XXX" où :
		- XXX est la clé de la file de message crée par le contrôleur

5. Et après ?
	- Pour gagner, il suffit simplement de se rendre à la sortie !
	- Si vous perdez, c'est que les minotaures vont ont eu !
	- Que vous gagnez ou que vous perdez, le jeu est sauvegarder en mémoire jusqu'a ce que le
		contrôleur soit relancer avec une nouvelle (ou la même) carte.


*** Note d'installation ***
----------------------------------------------------------------------------
Le jeu ne requiert aucune installation, juste l'obligation de la présence des
dossiers qu'ils utilisent : 
	- cartes : contient toutes les cartes créer avec l'éditeur
	- controleur : contient tous les lanceurs du jeu (controleur,joueur,minotaure)

Au cas où vous auriez un problème, l'exécutable removeAll permet de fermer les outils IPC (file
de message, segment de mémoire partagé et tableau de sémaphore). A utiliser seulement lorsque
le contrôleur rencontre une erreur où il ne les a pas fermer lui même.
"./removeAll XXX YYY ZZZ" où :
	- XXX est la clé de la file de message
	- YYY est la clé du segment de mémoire partagé
	- ZZZ est la clé du tableau de sémaphore
