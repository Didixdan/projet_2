/**
 * Classe carte qui créer un labyrinthe de 30 cases par 15 cases
 * L'entré est situé sur la case (0,8) 
 * La sortie est situé sur la case (29,8)
 * Premier clique = ajout mur invisible
 * Deuxième clique = ajout mur visible
 * Troisième clique = suppression mur
 * Choisir nombre de vies avec "+" et "-"
 *
 * @author Alexandre Varin & Florian Botte
 * @version 17/01/2019
 **/
#ifndef CARTE_H
#define CARTE_H

#include "ncurses.h"
#include <ncurses.h>    /* Pour printw, attron, attroff, COLOR_PAIR, getch */

#define LARGEURC 30 /* Correspond à la largeur max de la carte */
#define HAUTEURC 15 /* Correspond à la hauteur max de la carte */


/* renvoie la version de la carte ouverte */
int getVersion(int);

/* renvoie le nombre de vies restantes */
unsigned char getNbVieRestante(int);

/* modifier le nombre de vie restante de la sauvegarde */
void setVieRestante(int,unsigned char);

/* renvoie la position du valeureux guerrier */
void getPosGuerrier(int, unsigned char*,unsigned char*);

/* modifier la position du valeureux guerrier dans la sauvegarde */
void setPosGuerrier(int,int,int);

/* fonction qui duplique les données la carte vers la sauvegarde */
void dupDataCarte(int,int);

/* gere les mouvements du valeureux guerrier */
int bougerValGuerrier(int,int,int);

/* ouvre/créer une carte */
int editCarte(char *);

/* renvoie le nombre d'occurence de case dont la valeur est passé en paramètre */
int getNbVal(int,int);

/* crée la fenètre de la carte */
WINDOW * createWindowJeu(WINDOW *,int,int,int);

/* remplie la fenètre de la carte */
void remplireFenCarte(WINDOW *,int,int*,int*);

/* deplace le tête de lecture dans le fichier à l'endroit voulue (pour les cases) */
void deplaceInFileTo(int,int,int);

/* renvoie le nombre de vie de la carte */
unsigned char getNbVieDepart(int);

/* savoir on a perdu */
int hasLost(int);

/* savoir si on a gagner */
int hasWon(int);

/* ferme le fichier de la carte */
void closeCarte(int);
#endif