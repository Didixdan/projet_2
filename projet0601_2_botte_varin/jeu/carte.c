/**
 * Classe carte
 * @author Alexandre Varin & Florian Botte
 * @version 21/01/2019
 **/

#include <stdio.h>     /* Pour printf, scanf */
#include <fcntl.h>     /* Pour open */
#include <unistd.h>    /* Pour write */
#include <sys/stat.h>  /* Pour O_RDWR, O_CREAT, S_IRUSR, S_IWUSR */
#include <stdlib.h>    /* Pour exit, EXIT_SUCCESS, EXIT_FAILURE */
#include <ncurses.h>    /* Pour printw, attron, attroff, COLOR_PAIR, getch */
#include <string.h>     /* Pour strcmp, strtok */
#include <dirent.h>     /* Pour opendir, readdir, rewinddir, closedir */

#include "ncurses.h"
#include "carte.h"


/** 
 * Fonction qui gère le fait si on peut bouger le valeureux guerrier ou non 
 * @param fdCarte le descripteur de fichier de la carte
 * @param y position en y du deplacement voulu
 * @param x position en x du deplacement voulu
 * @return 
 * 0 si la case n'affecte pas le joueur
 * sinon la valeur de la case sur laquel le joueur veut aller
 */
int bougerValGuerrier(int fdCarte,int y, int x)
  {
  int ok=0;
  unsigned char valeur;

  deplaceInFileTo(fdCarte, y, x);
  if(read(fdCarte, &valeur, sizeof(unsigned char)) == (off_t)-1 )
    {
    perror("Erreur lors de la lecture de la valeur ");
    exit(EXIT_FAILURE);
    }
  ok=atoi((char*)&valeur);
  if(valeur > '0' && valeur < '4') 
    {
    if(valeur=='1')
      {
      deplaceInFileTo(fdCarte, y, x);
      valeur='3';
      /* on écrit la nouvelle valeur de la case */
      if(write(fdCarte, &valeur, sizeof(unsigned char)) == -1) {
        perror("Erreur write ");
        exit(EXIT_FAILURE);
        }
      }
    setVieRestante(fdCarte,getNbVieRestante(fdCarte)-1);
    }
  else if(valeur=='0')
    {
    setPosGuerrier(fdCarte,y,x);
    deplaceInFileTo(fdCarte,y,x);
    valeur='4';
    /* on écrit la nouvelle valeur de la case */
    if(write(fdCarte, &valeur, sizeof(unsigned char)) == -1) {
      perror("Erreur write ");
      exit(EXIT_FAILURE);
      }
    }

  return ok;
  }

int getVersion(int fdCarte)
  {
  int val;

  if(read(fdCarte, &val, sizeof(int)) == -1) {
    perror("Erreur lors de la lecture de la version ");
    exit(EXIT_FAILURE);
    }
  return val;
  } 

void dupDataCarte(int fdSrc, int fdDst)
  {
  int version;
  unsigned char nb_vies;
  unsigned char cases[LARGEURC*HAUTEURC];
  unsigned char posGuerrier[2] = {'8','0'};
  unsigned char depart='4';

  if(lseek(fdSrc, 0L, SEEK_SET) == (off_t)-1)
    {
    perror("Erreur lors du deplacement dans le fichier ");
    exit(EXIT_FAILURE);
    }

  if(read(fdSrc, &version, sizeof(int)) == (off_t)-1 )
    {
    perror("Erreur lors de la lecture de la valeur ");
    exit(EXIT_FAILURE);
    }

  if(read(fdSrc, &nb_vies, sizeof(unsigned char)) == (off_t)-1 )
    {
    perror("Erreur lors de la lecture de la valeur ");
    exit(EXIT_FAILURE);
    }

  if(read(fdSrc, &cases, sizeof(unsigned char)*(LARGEURC*HAUTEURC)) == (off_t)-1 )
    {
    perror("Erreur lors de la lecture de la valeur ");
    exit(EXIT_FAILURE);
    }

  close(fdSrc);
  /* on fait la sauvegarde */
  /* on écrit la version */
  if(write(fdDst, &version, sizeof(int)) == -1) {
      perror("Erreur write ");
      exit(EXIT_FAILURE);
    }
  /* on écrit le nombre de vies de départ */
  if(write(fdDst, &nb_vies, sizeof(unsigned char)) == -1) {
      perror("Erreur write ");
      exit(EXIT_FAILURE);
    }
  /* on écrit toutes les cases */
  if(write(fdDst, &cases, sizeof(unsigned char)*(LARGEURC*HAUTEURC)) == -1) {
      exit(EXIT_FAILURE);
    }
  /* on écrit le nombres de vies restantes (égal au nombre de vies de départ pour la première ouverture) */
  if(write(fdDst, &nb_vies, sizeof(unsigned char)) == -1) {
      perror("Erreur write ");
      exit(EXIT_FAILURE);
    }
  /* on écrit la position du guerrier */
  if(write(fdDst, &posGuerrier, sizeof(unsigned char)*2) == -1) { 
      perror("Erreur write ");
      exit(EXIT_FAILURE);
    }

  deplaceInFileTo(fdDst,8,0);
  /* on set la case de départ comme déjà visité (heureusement d'ailleurs) */
  if(write(fdDst, &depart, sizeof(unsigned char)) == -1) { 
      perror("Erreur write ");
      exit(EXIT_FAILURE);
    }
  }

unsigned char getNbVieRestante(int fdCarte)
  {
  unsigned char nb_vies_restantes;
  if(lseek(fdCarte,sizeof(int)+sizeof(unsigned char)+(sizeof(unsigned char)*(LARGEURC*HAUTEURC)),SEEK_SET) == (off_t)-1)
    {
      perror("Erreur lors du deplacement dans le fichier ");
      exit(EXIT_FAILURE);
    }
  if(read(fdCarte, &nb_vies_restantes, sizeof(unsigned char)) == -1) {
    perror("Erreur lors de la lecture de la version ");
    exit(EXIT_FAILURE);
    }
  return nb_vies_restantes;
  }

void setVieRestante(int fdCarte,unsigned char nbVie)
  {
    if(lseek(fdCarte, sizeof(int)+sizeof(unsigned char)+(sizeof(unsigned char)*(LARGEURC*HAUTEURC)),SEEK_SET) == (off_t)-1)
    {
      perror("Erreur lors du deplacement dans le fichier ");
      exit(EXIT_FAILURE);
    }
    if(write(fdCarte, &nbVie, sizeof(unsigned char)) == (off_t)-1 )
    {
    perror("Erreur lors de la lecture de la valeur ");
    exit(EXIT_FAILURE);
    }

  }

void setPosGuerrier(int fdCarte,int y,int x)
  {
    unsigned char posX = x + '0';
    unsigned char posY = y + '0';

  
    if(lseek(fdCarte, sizeof(int)+sizeof(unsigned char)+(sizeof(unsigned char)*(LARGEURC*HAUTEURC))+sizeof(unsigned char),SEEK_SET) == (off_t)-1)
    {
      perror("Erreur lors du deplacement dans le fichier ");
      exit(EXIT_FAILURE);
    }

    if(write(fdCarte, &posY, sizeof(unsigned char)) == (off_t)-1 )
    {
    perror("Erreur lors de la lecture de la valeur ");
    exit(EXIT_FAILURE);
    }

    if(write(fdCarte, &posX, sizeof(unsigned char)) == (off_t)-1 )
    {
    perror("Erreur lors de la lecture de la valeur ");
    exit(EXIT_FAILURE);
    }
  }

void getPosGuerrier(int fdCarte, unsigned char *y,unsigned char *x)
  {
    if(lseek(fdCarte,sizeof(int)+sizeof(unsigned char)+(sizeof(unsigned char)*(LARGEURC*HAUTEURC))+sizeof(unsigned char),SEEK_SET) == (off_t)-1)
    {
      perror("Erreur lors du deplacement dans le fichier ");
      exit(EXIT_FAILURE);
    }

    if(read(fdCarte, &(*y), sizeof(unsigned char)) == -1) {
    perror("Erreur lors de la lecture de la position Y du guerrier ");
    exit(EXIT_FAILURE);
    }

    if(read(fdCarte, &(*x), sizeof(unsigned char)) == -1) {
    perror("Erreur lors de la lecture de la position X du guerrier  ");
    exit(EXIT_FAILURE);
    }

  }

/**
  * Créer/Charge la carte dont le nom est passer en paramètre pour édition/jeu
  * @param nomCarte le nom de la carte à créer/charger
  * @return l'entier du descripteur du fichier
  **/
int editCarte(char * nomCarte)
	{
  DIR *dirp;                            /* Le repertoire */
  struct dirent *dp;                    /* Structure entree repertoire */

  int hasSave=0;                        /* booleen pour savoir si la carte possède une sauvegarde */
	int fdCarte=0;                        /* le descripteur de fichier de la carte */
  int fdSave=0;                         /* le descripteur de fichier de la sauvegarde */

  char pathCartes[256] = "../cartes/";  /* chemin vers le dossier carte */
  char pathSaves[256] = "../saves/";    /* chemin vers le dossier saves */

  char * nomFileSave;                   /* nom fichier sauvegarde */
  char version[sizeof(int)*8];          /* numéro de la version (pour creation sauvegarde) */
  char * fullpath;                      /* chemin entier vers le fichier de la carte ouverte */

  if(strchr(nomCarte,'_')) {
    /* le joueur passe une possible sauvegarde */
    fullpath = realpath(pathSaves,NULL);
    strcat(nomCarte,"_");
    strcat(nomCarte,"game");
    strcat(fullpath,"/");
    strcat(fullpath,nomCarte);
    if((fdCarte = open(fullpath, O_RDWR, S_IRUSR|S_IWUSR)) == -1) {
        fprintf(stderr, "Erreur lors de l'ouverture de la sauvegarde '%s' : la sauvegarde n'existe pas.\n", nomCarte);
        exit(EXIT_FAILURE);
    }
    return fdCarte;
  }
  /* ouverture du répértoire des sauvegarde pour voir si une sauvegarde existe */
  
  if((dirp = opendir(pathSaves)) == NULL) {
    fprintf(stderr, "Probleme lors de l'ouverture du repertoire '%s'. ", pathSaves);
    perror("Erreur ");
    exit(EXIT_FAILURE);
  }
  

  /* parcours du contenu du repertoire */

  while((dp = readdir(dirp)) != NULL) {
    if((strcmp(dp->d_name, ".") != 0) && (strcmp(dp->d_name, "..") != 0)) {
      nomFileSave = (char*)malloc(sizeof(char)*strlen(dp->d_name)+1);
      if(nomFileSave==NULL){
        perror("Erreur lors de l'allocation mémoire du nom de la save ");
        exit(EXIT_FAILURE);
      }
      strcpy(nomFileSave,dp->d_name);
      if(strcmp(strtok(dp->d_name,"_"), nomCarte)==0) {
        hasSave=1;
        break;
      }
    }
  }

  if(hasSave)
    {
    /* on ouvre le fichier de sauvegarde */
    strcat(pathSaves,nomFileSave);
    fullpath = realpath(pathSaves, NULL);
    /* Ouverture du fichier de sauvegarde de la carte */
    if((fdCarte = open(fullpath, O_RDWR, S_IRUSR|S_IWUSR)) == -1) fdCarte=-1; /* sauvegarde n'existe pas */
    }
  else 
    {
    /* La carte n'a pas de fichier de sauvegarde, on regarde si elle existe dans carte
     * si oui on crée une sauvegarde 
     */
    strcat(pathCartes,nomCarte); 
    fullpath = realpath(pathCartes, NULL);
    /* Ouverture et/ou création du fichier de la carte */
    if((fdCarte = open(fullpath, O_RDWR, S_IRUSR|S_IWUSR)) == -1) fdCarte=-2; /* carte n'existe pas */
    else 
      {
      /* on construit le chemin du fichier de sauvegarde */
      fullpath = realpath(pathSaves, NULL);

      strcat(nomCarte,"_");
      sprintf(version,"%d",getVersion(fdCarte));
      strcat(nomCarte,version);
      strcat(nomCarte,"_");
      strcat(nomCarte,"game");
      strcat(fullpath,"/");
      strcat(fullpath,nomCarte);

      if((fdSave = open(fullpath, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR)) == -1) 
        {
        fprintf(stderr, "Erreur lors de la creation du fichier de sauvegarde pour la carte '%s'\n", nomCarte);
        perror(" ");
        exit(EXIT_FAILURE);
        }

      dupDataCarte(fdCarte,fdSave);
      return fdSave;
      }
    }

	return fdCarte;
	}


int getNbVal(int fdCarte,int valeurCase)
  {
  int i,res,nb=0;
  unsigned char val;
  
  if(lseek(fdCarte, sizeof(int)+sizeof(unsigned char), SEEK_SET) == (off_t)-1) /* deplacement au case */
    {
      perror("Erreur lors du deplacement dans le fichier ");
      exit(EXIT_FAILURE);
    }
  for(i=0;i<(LARGEURC*HAUTEURC);i++) {
    res = read(fdCarte, &val, sizeof(unsigned char));
    if(res == -1) {
      fprintf(stderr, "Erreur lors de la lecture de la valeur %d", i);
      perror(" ");
      exit(EXIT_FAILURE);
    } 
    if(res != 0) {
        if(val-'0'==valeurCase) nb++;
    }
  }
  /*printf("nb=%d", nb);*/
  return nb;
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
void remplireFenCarte(WINDOW *carte,int fdCarte,int *vPosY,int *vPosX)
    {
    /* Lecture de tout le fichier */
    unsigned char val;
    unsigned char y,x=y='0';
    int i=0,j=0,res;
    int k=0;
    deplaceInFileTo(fdCarte,0,0);
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
            wattron(carte,COLOR_PAIR(val-'0'+10));
            mvwaddch(carte,j,i,val);
            wattroff(carte,COLOR_PAIR(val-'0'));
            i++;
            k++;
        }
    } while(res != 0 && k != 450);

    wattron(carte,COLOR_PAIR(1));
    mvwaddch(carte, 8, 29, 'S');
    wattron(carte,COLOR_PAIR(1));

    getPosGuerrier(fdCarte,&y,&x);
    *vPosX=x-'0';
    *vPosY=y-'0';
    wattron(carte,COLOR_PAIR(9));
    mvwaddch(carte, y-'0', x-'0', ' ');
    wattron(carte,COLOR_PAIR(9));

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
  if(lseek(fdCarte, sizeof(int)+sizeof(unsigned char)+sizeof(unsigned char)*pos, SEEK_SET) == (off_t)-1) /* retour au début des cases */
    {
    perror("Erreur lors du deplacement dans le fichier ");
    exit(EXIT_FAILURE);
    }
  }

/**
  * Renvoie le nombre de vie de la carte passé en paramètre
  * @param fdCarte le descripteur de fichier de la carte
  **/
unsigned char getNbVieDepart(int fdCarte)
  {
  unsigned char val='0';

  if(lseek(fdCarte, sizeof(int), SEEK_SET) == (off_t)-1) /* déplacement après la version */
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

int hasLost(int fdCarte)
  {
  return getNbVieRestante(fdCarte)=='0';
  }

int hasWon(int fdCarte) 
  {
  unsigned char y,x=y='0';
  getPosGuerrier(fdCarte,&y,&x);
  return (x-'0'==29&&y-'0'==8);
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