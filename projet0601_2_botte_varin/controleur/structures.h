#ifndef _STRUCTURES_
#define _STRUCTURES_


/* type des acteurs */
#define TYPE_JOUEUR 1
#define TYPE_MINOTAURE 2

/* Type de message requete */
#define TYPE_REQ_SMPTS  1
#define TYPE_REQ_DECO   2

/* Type de message reponse */
#define TYPE_REP_SMPTS 1

#include <sys/types.h>

typedef struct {
  unsigned char type;
} case_t;

typedef struct {
  int largeur;
  int hauteur;
  unsigned char nbVie;
  case_t * cases;
}terrain_t;


typedef struct {
  int * largeur;
  int * hauteur;
  unsigned char * nbVie;
  case_t * cases;
}segment_t;


/* a modifier */
typedef struct {
  int cleSMP;
  int cleTS;
} reponse_smpts_t;

typedef struct {
  long type;
  int typeActeur;
  pid_t pid;
}requete_t;

typedef struct {
  long type;
  union {
    reponse_smpts_t r1;
  }rep;
}reponse_t;



#endif