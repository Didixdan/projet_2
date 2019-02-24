#ifndef _STRUCTURES_
#define _STRUCTURES_


/* type des acteurs */
#define TYPE_JOUEUR 1
#define TYPE_MINOTAURE 2

/* Type de message requete */
#define TYPE_REQ_SMPTS  1 /* avoir les clé smp et ts */
#define TYPE_REQ_DECO   2 /* requête pour se déconecter */
#define TYPE_REQ_EVT    3 /* requête lorsque le joueur envoie un événement (gagner ou perdu) */

/* Type de message reponse */
#define TYPE_REP_SMPTS  1
#define TYPE_REP_EVT    2

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
  int nbMino;
} reponse_evt_t;

typedef struct {
  long type;
  int typeActeur;
  pid_t pid;
}requete_t;

typedef struct {
  long type;
  union {
    reponse_smpts_t r1;
    reponse_evt_t r2;
  }rep;
}reponse_t;



#endif