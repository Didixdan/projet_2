/**
 * Classe carte
 * @author Alexandre Varin & Florian Botte
 * @version 17/01/2019
 **/


#include "carte.h"

/**
#TODO#
On ne peut pas se déplacer sur des cases déjà visités et quitter sinon le position du guerrier est faussé
*/


/* Permet d'écrire la carte ouverte dans le segment de mémoire partagé */
terrain_t getTerrainCarte(int fdCarte) {
  unsigned char val;
  case_t uneCase;
  int i;
  int res;
  terrain_t terrain;
  case_t * lesCases;
  unsigned char nbVie;

  terrain.largeur = LARGEURC;
  terrain.hauteur = HAUTEURC;
  i=res=0;
  lesCases = (case_t*)malloc(sizeof(case_t)*(LARGEURC*HAUTEURC));

  if(lseek(fdCarte, sizeof(int), SEEK_SET) == (off_t)-1) /* retour au debut du fichier après la version */
    {
    perror("Erreur lors du deplacement dans le fichier ");
    exit(EXIT_FAILURE);
    }
  if(read(fdCarte, &nbVie, sizeof(unsigned char)) == -1) {
    perror("Erreur lors de la lecture du nbr de vie ");
    exit(EXIT_FAILURE);
    }
  terrain.nbVie = nbVie;
  do {
    res = read(fdCarte, &val, sizeof(unsigned char));
    if(res == -1) {
      fprintf(stderr, "Erreur lors de la lecture de la valeur %d", i);
      perror(" ");
      exit(EXIT_FAILURE);
    }
    if(res != 0) {
      /* ajout au cases */
      if(i==240) uneCase.type = ZERO_ASCII+10;
      else uneCase.type = val;
      lesCases[i]=uneCase;
      i++;
      }
    } while(res != 0 && i != 450);
    
  terrain.cases = lesCases;
  return terrain;
}
  
  
void* getAdresseSMP(int shmid) {
    void* adresse;
    /* Recuperation du segment de memoire partagee */
    if((adresse = shmat(shmid, NULL, 0)) == (void*)-1) {
        perror("Erreur lors de l'attachement du segment de memoire partagee");
        exit(EXIT_FAILURE);
    }
    return adresse;
}

segment_t * creer_segment(key_t cle,terrain_t terrain) {
    int shmid;
    void * adresse;
    segment_t * result = (segment_t*)malloc(sizeof(result));
    if((shmid = shmget(cle, sizeof(int)*2+sizeof(unsigned char)+sizeof(case_t)*terrain.largeur*terrain.hauteur, S_IRUSR | S_IWUSR | IPC_CREAT | IPC_EXCL)) == -1) {
        if(errno == EEXIST)
            fprintf(stderr, "Le segment de memoire partagee (cle=%d) existe deja\n", cle);
        else
            perror("Erreur lors de la creation du segment de memoire ");
        exit(EXIT_FAILURE);
    }

    /* Attachement du segment de memoire partagee */
    adresse = getAdresseSMP(shmid);

    result->largeur=(int*)adresse;
    result->hauteur=&result->largeur[1];
    result->nbVie=(unsigned char*)&result->hauteur[1];
    result->cases=(case_t *)&result->nbVie[1];

    /* initialisation du segment */
    *(result->largeur)=terrain.largeur;
    *(result->hauteur)=terrain.hauteur;
    *(result->nbVie)=terrain.nbVie;
    memcpy(result->cases,terrain.cases,sizeof(case_t)*terrain.largeur*terrain.hauteur);
    
    return result;
}

void setSegmentVals(int shmid,segment_t *seg) {
  void *adresse;
  segment_t * result = (segment_t*)malloc(sizeof(result));
  adresse = getAdresseSMP(shmid);

  result->largeur=(int*)adresse;
  result->hauteur=&result->largeur[1];
  result->nbVie=(unsigned char*)&result->hauteur[1];
  result->cases=(case_t *)&result->nbVie[1];


  *(result->largeur)=*(seg->largeur);
  *(result->hauteur)=*(seg->hauteur);
  *(result->nbVie)=*(seg->nbVie);
  memcpy(result->cases,seg->cases,sizeof(case_t)*(*seg->largeur)*(*seg->hauteur));
}

int segmentEquals(segment_t *seg1, segment_t *seg2) {
  int ok=1/*,i*/;
  printf("bjr");
  /*for(i=0;i<(LARGEURC*HAUTEURC);i++) {
    if(seg1->cases[i].type!=seg2->cases[i].type) {ok=0;break;}
  }*/
  return ok;
}

segment_t * getSegmentVals(int shmid) {
  void* adresse;
  segment_t * result = (segment_t*)malloc(sizeof(result));
  adresse = getAdresseSMP(shmid);
  
  result->largeur=(int*)adresse;
  result->hauteur=&result->largeur[1];
  result->nbVie=(unsigned char*)&result->hauteur[1];
  result->cases=(case_t *)&result->nbVie[1];

  return result;
}


int getNbVal(int shmid,int valeurCase)
  {
  int i,nb=0;
  unsigned char val;
  segment_t *result = getSegmentVals(shmid);
  for(i=0;i<(LARGEURC*HAUTEURC);i++) {
    val = result->cases[i].type;
    if(val-ZERO_ASCII==valeurCase) nb++;
    }
  return nb;
  }

/**
  * Charge la carte dont le nom est passer en paramètre pour édition/jeu
  * @param nomCarte le nom de la carte à créer/charger
  * @return l'entier du descripteur du fichier
  **/
int openCarte(const char * nomCarte)
	{
	int fdCarte=0;
  char path[30] = "../cartes/";
  char * fullpath;
  strcat(path,nomCarte);
  fullpath = realpath(path, NULL);

	/* Ouverture et/ou création du fichier de la carte */
	if((fdCarte = open(fullpath, O_RDWR, S_IRUSR|S_IWUSR)) == -1) {
		fprintf(stderr,"La carte '%s' n'existe pas.\n",nomCarte);
    exit(EXIT_FAILURE);
		}
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
WINDOW * createWindowJeu(WINDOW *fenMere,int y,int x)
	{
	WINDOW * carte;
	carte = derwin(fenMere,HAUTEURC+1, LARGEURC+1, y, x); /* +1 pour le décalage avec la fenetre mere (bien avoir 15 lignes et 30 colonnes) */
	return carte;
	}

/**
  * Remplie la fenetre passer en paramètre par les couleurs des cases
  * @param carte la fenetre de la car--te à modifier
  * @param fdCarte le descripteur de fichier
  * @return l'entier du descripteur du fichier
  **/
void remplireFenCarte(WINDOW *carte,int shmid,int *vPosY,int *vPosX)
    {
    unsigned char val;
    unsigned char y,x=y=ZERO_ASCII;
    int i=0,j=0;
    int k=0;
    segment_t *leSegment = getSegmentVals(shmid);
    do {
      if(i==30) {
          j++;
          i=0;
          }
      val = leSegment->cases[k].type;
      /*if(val==ZERO_ASCII+10)printf("(%c;%d)",val,k);*/
      wattron(carte,COLOR_PAIR(val-ZERO_ASCII+10));
      mvwaddch(carte,j,i,val);
      wattroff(carte,COLOR_PAIR(val-ZERO_ASCII));
      i++;
      k++;
    } while(k != 450);

    wattron(carte,COLOR_PAIR(1));
    mvwaddch(carte, 8, 29, 'S');
    wattron(carte,COLOR_PAIR(1));

    getPosGuerrier(shmid,&y,&x);
    *vPosX=x-'0';
    *vPosY=y-'0';
    wattron(carte,COLOR_PAIR(9));
    mvwaddch(carte, *vPosY, *vPosX, ' ');
    wattron(carte,COLOR_PAIR(9));

    }

void setValCase(int shmid,int y,int x,unsigned char val) {
  int pos=y==0?x:y*30+x;
  segment_t *result = getSegmentVals(shmid);
  result->cases[pos].type = val;
  setSegmentVals(shmid,result);
}

void getPosGuerrier(int shmid, unsigned char *y,unsigned char *x)
  {
  int i,j,k=j=i=0;
  unsigned char val;
  segment_t *result = getSegmentVals(shmid);
  do {
    if(i==30) {
      j++;
      i=0;
      }
    val = result->cases[k].type;
    if(val-ZERO_ASCII==10) {
      *y=j+'0';
      *x=i+'0';
    }
    i++;
    k++;
    }while(k!=450);
  }
/**
  * Renvoie le nombre de vie de la carte passé en paramètre
  * @param fdCarte le descripteur de fichier de la carte
  **/
unsigned char getNbVie(int shmid)
  {
  segment_t *result = getSegmentVals(shmid);
  return *result->nbVie;
  }
 
void setVieRestante(int shmid,unsigned char nbVie)
  {
  segment_t *result = getSegmentVals(shmid);
  result->nbVie = &nbVie;
  setSegmentVals(shmid,result);
  }

void setPosGuerrier(int shmid,int y,int x)
  {
  segment_t *result = getSegmentVals(shmid);
  int pos=y==0?x:y*30+x;
  /*printf("(%d;%d;%c)",y,x,result->cases[pos].type);*/
  result->cases[pos].type = '0'+10;
  setSegmentVals(shmid,result);
  }

/** 
 * Fonction qui gère le fait si on peut bouger le valeureux guerrier ou non 
 * @param fdCarte le descripteur de fichier de la carte
 * @param y position en y du deplacement voulu
 * @param x position en x du deplacement voulu
 * @return 
 * 0 si la case n'affecte pas le joueur
 * sinon la valeur de la case sur laquel le joueur veut aller
 */
int bougerValGuerrier(int shmid,int y, int x)
  {
  segment_t *result = getSegmentVals(shmid);
  int pos=y==0?x:y*30+x;
  unsigned char valeur = result->cases[pos].type;
  int ok=atoi((char*)&valeur);
  if(valeur=='1')
    {
    valeur='3';
    }
  else if(valeur=='0' || valeur=='4')
    {
    setPosGuerrier(shmid,y,x);
    }
  else if(valeur>'0'+10)
    {
    /* méchant minautore */
    }
  return ok;
  }

void setCaseTypeMinotaure(int shmid,int semid){
	segment_t *result = getSegmentVals(shmid);	
	int ligne = 0;
	int colonne = 0;

  unsigned short valeur;

  unsigned char type='0'+11;
	int positionCase = 0;	
	srand(time(NULL));
	
	do{	
		ligne = rand()%HAUTEURC;	
		colonne = rand()%LARGEURC;	
		positionCase = ligne==0?colonne:ligne*30+colonne;
		/*printf("%d\n",result->cases[positionCase].type));*/
		/*printf("poscase : %d\n", positionCase);*/
	}while((result->cases[positionCase].type) != 48);


 /* Recuperation de la valeur du semaphore */
  if((valeur = semctl(semid, 0, GETVAL)) == -1) {
    perror("Erreur lors de la recuperation de la valeur du semaphore ");
    exit(EXIT_FAILURE);
  }
  
  switch(valeur) {
    case 4:
      type = '0'+11;
      break;
    case 3:
      type = '0'+12;
      break;
    case 2:
      type = '0'+13;
      break;
    case 1:
      type = '0'+14;
      break;
    case 0:
      type = '0'+15;
      break;
  }

	result->cases[positionCase].type = type;
  setSegmentVals(shmid,result);

	printf("%c\n",result->cases[positionCase].type);
}

void makeMinotaurAppear(WINDOW* fen, int shmid, int posCase){

}

int hasLost(int shmid)
  {
    return 0;
  }

int hasWon(int shmid) 
  {
    return 0;

  }

void closeCarte(int fdCarte)
  {
  if(close(fdCarte) == -1) {
      perror("Erreur close ");
      exit(EXIT_FAILURE);
    }
  }

  /**
  * Ferme le fichier de la carte
  * @param fdCarte le descripteur de fichier de la carte
  **/
