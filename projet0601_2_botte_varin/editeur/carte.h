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

#define _XOPEN_SOURCE 500

/* crée une carte par défaut */
void createDefaultCarte(int);

/* incremente la version de la carte de 1 */
void incrementeVersion(int);

/* renvoie la version de la carte ouverte */
int getVersion(int);

/* ouvre/créer une carte */
int editCarte(const char *);

/* crée la fenètre de la carte */
WINDOW * createWindowJeu(WINDOW *,int,int,int);

/* remplie la fenètre de la carte */
void remplireFenCarte(WINDOW *,int);

/* deplace le tête de lecture dans le fichier à l'endroit voulue (pour les cases) */
void deplaceInFileTo(int,int,int);

/* gère le clique sur la carte */
void cliqueCarte(WINDOW *,int,int,int);

/* renvoie le nombre de vie de la carte */
unsigned char getNbVie(int);

/* set le nombre de vie de la carte */
void setNbVie(int,unsigned char);

/*permet de modifier le nombre de vies*/
void cliqueVie(int,int);

/* ferme le fichier de la carte */
void closeCarte(int);

#endif