/**
 * Classe carte
 * @author Alexandre Varin & Florian Botte
 * @version 17/01/2019
 **/

#include <string.h>
#include <stdio.h>     /* Pour printf, scanf */
#include <fcntl.h>     /* Pour open */
#include <unistd.h>    /* Pour write */
#include <sys/stat.h>  /* Pour O_WRONLY, O_CREAT, S_IRUSR, S_IWUSR */
#include <stdlib.h>    /* Pour exit, EXIT_SUCCESS, EXIT_FAILURE */
#include <ncurses.h>    /* Pour printw, attron, attroff, COLOR_PAIR, getch */
#include <string.h>

#include "ncurses.h"
#include "carte.h"


/**
  * Créer une carte par defaut (0 sur toutes les cases, 1 vie et version 0)
  * @param fdCarte le descripteur de fichier de la carte
  **/
void createDefaultCarte(int fdCarte)
	{
	int sizeCarte = LARGEURC*HAUTEURC;
	int i;  /* temporaire */
	int version=0;
  unsigned char nbVie='1';
  unsigned char lesCases[LARGEURC*HAUTEURC];
	for(i=0;i<sizeCarte;i++) lesCases[i]='0';

	/* on écrit la version de la carte */
	if(write(fdCarte, &version, sizeof(int)) == -1) {
    	perror("Erreur write ");
    	exit(EXIT_FAILURE);
    }

    /* on écrit le nombre de vie de la carte */
	if(write(fdCarte, &nbVie, sizeof(unsigned char)) == -1) {
    	perror("Erreur write ");
    	exit(EXIT_FAILURE);
    }

  /* on écrit les cases de la carte*/
  if(write(fdCarte, &lesCases, sizeof(unsigned char)*strlen((char*)lesCases)) == -1) {
    perror("Erreur write ");
    exit(EXIT_FAILURE);
  }
	}

void incrementeVersion(int fdCarte)
  {
  int version = 0;
  if(lseek(fdCarte, 0L, SEEK_SET) == (off_t)-1) /* retour au debut du fichier */
    {
    perror("Erreur lors du deplacement dans le fichier ");
    exit(EXIT_FAILURE);
    }
  if(read(fdCarte, &version, sizeof(int)) == (off_t)-1) /* on lit l'ancienne version */
    {
    perror("Erreur lors du deplacement dans le fichier ");
    exit(EXIT_FAILURE);
    }

  version++;

  if(lseek(fdCarte, 0L, SEEK_SET) == (off_t)-1) /* retour au debut du fichier */
    {
    perror("Erreur lors du deplacement dans le fichier ");
    exit(EXIT_FAILURE);
    }
  if(write(fdCarte, &version, sizeof(int)) == -1) { /* on écrit la nouvelle version */
      perror("Erreur write ");
      exit(EXIT_FAILURE);
    }
  }

int getVersion(int fdCarte)
  {
  int val;
  if(lseek(fdCarte, 0L, SEEK_SET) == (off_t)-1) /* retour au debut du fichier */
    {
    perror("Erreur lors du deplacement dans le fichier ");
    exit(EXIT_FAILURE);
    }
  if(read(fdCarte, &val, sizeof(int)) == -1) {
    perror("Erreur lors de la lecture de la version ");
    exit(EXIT_FAILURE);
    }
  return val;
  } 
/**
  * Créer/Charge la carte dont le nom est passer en paramètre pour édition/jeu
  * @param nomCarte le nom de la carte à créer/charger
  * @return l'entier du descripteur du fichier
  **/
int editCarte(const char * nomCarte)
	{
	int fdCarte=0;
  char pathVanilla[30] = "../cartes/";
  char pathCpy[30] = "../cartes/";
  char * fullpath;
  strcat(pathCpy,nomCarte);
  fullpath = realpath(pathCpy, NULL);

	/* Ouverture et/ou création du fichier de la carte */
	if((fdCarte = open(fullpath, O_RDWR, S_IRUSR|S_IWUSR)) == -1)
		{
		/* si on est ici la carte n'existe pas */
    /* création du lien de la futur carte */
    fullpath = realpath(pathVanilla, NULL);
    strcat(fullpath,"/");
    strcat(fullpath,nomCarte);

  	if((fdCarte = open(fullpath, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR)) == -1) {
      fprintf(stderr,"Probleme d'ouverture du fichier '%s' au chemin %s.",nomCarte,fullpath);
  		perror("Erreur ");
  		exit(EXIT_FAILURE);
  		}
  	createDefaultCarte(fdCarte);
		}
  incrementeVersion(fdCarte);
	return fdCarte;
	}

/**
  * Créer la fenètre de la carte
  * @param fenMere la fenetre mère de la carte
  * @param fdCarte le descripteur de fichier
  * @param y position Y par rapport à la fenetre mère
  * @param x position X par rapport à la fenetre mère
  * @return la fenetre de la carte créer
  **/
WINDOW * createWindowJeu(WINDOW *fenMere,int fdCarte,int y,int x)
	{
	WINDOW * carte;
	if(lseek(fdCarte, sizeof(int)+sizeof(unsigned char), SEEK_SET) == (off_t)-1) 
		{
  	perror("Erreur lors du deplacement dans le fichier ");
  	exit(EXIT_FAILURE);
		}
	carte = derwin(fenMere,HAUTEURC+1, LARGEURC+1, y, x); /* +1 pour le décalage avec la fenetre mere (bien avoir 15 lignes et 30 colonnes) */
	return carte;
	}

/**
  * Remplie la fenetre passer en paramètre par les couleurs des cases
  * @param carte la fenetre de la carte à modifier
  * @param fdCarte le descripteur de fichier
  * @return l'entier du descripteur du fichier
  **/
void remplireFenCarte(WINDOW *carte,int fdCarte)
    {
    /* Lecture de tout le fichier */
    char val;
    int i=0,j=0,res;
    int k=0;
    do {
        res = read(fdCarte, &val, sizeof(unsigned char));
        if(res == -1) {
          fprintf(stderr, "Erreur lors de la lecture de la valeur %d", i);
          perror(" ");
          exit(EXIT_FAILURE);
        }
        if(res != 0) {
            if(i==30)
                {
                j++;
                i=0;
                }
            wattron(carte,COLOR_PAIR(atoi(&val)+10));
            mvwaddch(carte,j,i,val);
            wattroff(carte,COLOR_PAIR(atoi(&val)+10));
            i++;
            k++;
        }
    } while(res != 0);
    wattron(carte,COLOR_PAIR(9));
    mvwaddch(carte, 8, 0, ' ');
    wattron(carte,COLOR_PAIR(9));

    wattron(carte,COLOR_PAIR(1));
    mvwaddch(carte, 8, 29, 'S');
    wattron(carte,COLOR_PAIR(1));
    }

/**
  * Deplace le curseur du fichier vers la case y*x pour lecture/ecriture
  * @param fdCarte le descripteur de fichier de la carte
  * @param y position Y de la case
  * @param x position X de la case
  **/
void deplaceInFileTo(int fdCarte,int y,int x)
  {
  int pos;
  pos=y==0?x:y*30+x;
  if(lseek(fdCarte, 0L, SEEK_SET) == (off_t)-1) /* retour au debut du fichier */
    {
    perror("Erreur lors du deplacement dans le fichier ");
    exit(EXIT_FAILURE);
    }
  if(lseek(fdCarte, sizeof(int)+sizeof(unsigned char), SEEK_CUR) == (off_t)-1) /* retour au début des cases */
    {
    perror("Erreur lors du deplacement dans le fichier ");
    exit(EXIT_FAILURE);
    }
  if(lseek(fdCarte, sizeof(unsigned char)*pos, SEEK_CUR) == (off_t)-1) /* déplacement sur la case */
    {
    perror("Erreur lors du deplacement dans le fichier ");
    exit(EXIT_FAILURE);
    }
  }

/**
  * Renvoie le nombre de vie de la carte passé en paramètre
  * @param fdCarte le descripteur de fichier de la carte
  **/
unsigned char getNbVie(int fdCarte)
  {
  unsigned char val;
  if(lseek(fdCarte, 0L, SEEK_SET) == (off_t)-1) /* retour au debut du fichier */
    {
    perror("Erreur lors du deplacement dans le fichier ");
    exit(EXIT_FAILURE);
    }
  if(lseek(fdCarte, sizeof(int), SEEK_CUR) == (off_t)-1) /* déplacement après la version */
    {
    perror("Erreur lors du deplacement dans le fichier ");
    exit(EXIT_FAILURE);
    }
  if(read(fdCarte, &val, sizeof(unsigned char)) == -1) {
    perror("Erreur lors de la lecture du nombre de vie ");
    exit(EXIT_FAILURE);
    }
  return val;
  }

/**
  * Set le nombre de vie de la carte passé en paramètre
  * @param fdCarte le descripteur de fichier de la carte
  **/
void setNbVie(int fdCarte,unsigned char nbVie)
  {
  if(lseek(fdCarte, 0L, SEEK_SET) == (off_t)-1) /* retour au debut du fichier */
    {
    perror("Erreur lors du deplacement dans le fichier ");
    exit(EXIT_FAILURE);
    }
  if(lseek(fdCarte, sizeof(int), SEEK_CUR) == (off_t)-1) /* déplacement après la version */
    {
    perror("Erreur lors du deplacement dans le fichier ");
    exit(EXIT_FAILURE);
    }
  if(write(fdCarte, &nbVie, sizeof(unsigned char)) == -1) {
    perror("Erreur lors de l'écriture du nombre de vie ");
    exit(EXIT_FAILURE);
    }
  }

/**
  * Gère le clique sur une case de la carte pour modification
  * @param carte la fenetre de la carte à modifier
  * @param y position Y du clique
  * @param x position X du clique
  **/
void cliqueCarte(WINDOW * carte,int fdCarte,int y,int x)
  {
  unsigned char temp = mvwinch(carte,y,x);
  temp++;
  if((y==8 && x==0) || (y==8 && x==29)) return; /* si on touche à l'entrée ou la sortie, on fait rien */
  if(temp=='3')temp='0'; /* si la case a passer toutes les étapes, on reviens à la case vide */
  /* on modifie le visuel */
  wattron(carte,COLOR_PAIR(atoi((char*)&temp)+10));
  mvwaddch(carte,y,x,temp);
  wattroff(carte,COLOR_PAIR(atoi((char*)&temp)+10));
  /* on modifie le fichier */
  deplaceInFileTo(fdCarte,y,x); /* on se déplace à la case */
  if(write(fdCarte, &temp, sizeof(unsigned char)) == -1) { /* on modifie la valeur de la case */
    perror("Erreur write ");
    exit(EXIT_FAILURE);
  }
  incrementeVersion(fdCarte);
  }

void cliqueVie(int fdCarte,int val)
  {
  unsigned char nbVie = getNbVie(fdCarte);
  if(val==0)
    {
    if((atoi((char*)&nbVie)+1==10))return;
    else setNbVie(fdCarte,nbVie+1);
    }
  else if(val==1) 
    {
    if((atoi((char*)&nbVie)-1==0))return;
    else setNbVie(fdCarte,nbVie-1);
    }
  incrementeVersion(fdCarte);
  }

/**
  * Ferme le fichier de la carte
  * @param fdCarte le descripteur de fichier de la carte
  **/
void closeCarte(int fdCarte)
  {
  if(close(fdCarte) == -1) {
      perror("Erreur close ");
      exit(EXIT_FAILURE);
    }
  }