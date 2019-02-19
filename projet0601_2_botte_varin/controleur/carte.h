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

#define LARGEURC 30 /* Correspond à la largeur max de la carte */
#define HAUTEURC 15 /* Correspond à la hauteur max de la carte */
#define ZERO_ASCII 48 /* valeur de 0 en ascii (pour l'affichage d'un unsigned char en int) */

#include <stdio.h>     /* Pour printf, scanf */
#include <fcntl.h>     /* Pour open */
#include <unistd.h>    /* Pour write */
#include <sys/stat.h>  /* Pour O_WRONLY, O_CREAT, S_IRUSR, S_IWUSR */
#include <stdlib.h>    /* Pour exit, EXIT_SUCCESS, EXIT_FAILURE */
#include <ncurses.h>    /* Pour printw, attron, attroff, COLOR_PAIR, getch */
#include <string.h>
#include <sys/shm.h>    /* Pour shmget, shmat, shmdt */
#include <sys/msg.h>    /* Pour msgget */
#include <sys/sem.h>  /* Pour semget, semctl, semop */
#include <errno.h>      /* Pour errno */

#include "ncurses.h"
#include "structures.h"
/* ouvre une carte */
int openCarte(const char *);

/* donne le terrain par rapport au fichier passer en paramètre */
terrain_t getTerrainCarte(int);

/* crée la fenètre de la carte */
WINDOW * createWindowJeu(WINDOW *,int,int);

/* remplie la fenètre de la carte */
void remplireFenCarte(WINDOW *,int ,int *,int *);

/* gere les mouvements du valeureux guerrier */
int bougerValGuerrier(int,int,int);

segment_t * creer_segment(key_t,terrain_t); 

segment_t *getSegmentVals(key_t);

void setSegmentVals(key_t,segment_t*);

int getNbVal(int,int);

void setValCase(int,int,int,unsigned char);

/* renvoie le nombre de vies restantes */
unsigned char getNbVie(int);

/* modifier le nombre de vie restante de la sauvegarde */
void setVieRestante(int,unsigned char);

/* renvoie la position du valeureux guerrier */
void getPosGuerrier(int, unsigned char*,unsigned char*);

/* modifier la position du valeureux guerrier dans la sauvegarde */
void setPosGuerrier(int,int,int);

/* savoir on a perdu */
int hasLost(int);

/* savoir si on a gagner */
int hasWon(int);

/* ferme le fichier de la carte */
void closeCarte(int);

#endif