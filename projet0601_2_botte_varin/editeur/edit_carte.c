
#include <stdio.h>     /* Pour printf, scanf */
#include <stdlib.h>    /* Pour exit, EXIT_SUCCESS, EXIT_FAILURE */

#include "carte.h"
#include "ncurses.h"

#define LARGEUR 75      /* Largeur de la fenêtre */
#define HAUTEUR 27      /* Hauteur de la fenêtre */
#define POSX    20      /* Position horizontale de la fenêtre */
#define POSY    5       /* Position verticale de la fenêtre */

int main(int argc, char* argv[])
	{
    int ch,posX;
	int fdCarte;
    int sourisX,sourisY;

    int infosFenY = 10; /* taille en Y de la fenetre */
    /** taille limite de clique sur la carte */
    int limitX = LARGEURC, limitY = HAUTEURC + infosFenY + 1;

	WINDOW *jeuCarte,*jeuFen, *infosFen,*infosText, *buttonFen;
    WINDOW *boutonPlus, *boutonMoins;

    if(argc!=2) {
        fprintf(stderr, "Usage: %s nomCarte\n", argv[0]);
        fprintf(stderr, "\tOu:\n");
        fprintf(stderr, "\t\tnomCarte : nom de la carte a ouvrir/creer\n");
        exit(EXIT_FAILURE);
    }

    /* Initialisation de ncurses */
    ncurses_initialiser();
    ncurses_souris();
    ncurses_couleurs();

    /* Vérification de la taille du terminale */
    if((COLS < POSX + LARGEUR) || (LINES < POSY + HAUTEUR)) {
      ncurses_stopper();
      fprintf(stderr, "Les dimensions du terminal sont insufisantes : l=%d,h=%d au lieu de l=%d,h=%d\n", COLS, LINES, POSX + LARGEUR, POSY + HAUTEUR);
      exit(EXIT_FAILURE);
    }



    /*Calcul de la position centrale*/

    fdCarte = editCarte(argv[1]);

    posX = COLS / 2 - 1;
    move(0,0);
    
    infosFen = newwin(infosFenY, posX, 0, 0);
    mvwaddch(infosFen, 0, 0, ' ');

    infosText = derwin(infosFen,8, posX-2,1,1);
    scrollok(infosText, TRUE);
    


    buttonFen = newwin(9,20,11,LARGEURC+3);
    wprintw(buttonFen,"Vies : %c",getNbVie(fdCarte));
    wbkgd(buttonFen,COLOR_PAIR(15));

    boutonPlus = derwin(buttonFen,3,6,1,0);
    boutonMoins = derwin(buttonFen,3,6,1,6);

    mvwaddch(boutonPlus,1,3,'+');
    mvwaddch(boutonMoins,1,3,'-');

    wbkgd(boutonPlus,COLOR_PAIR(15));
    wbkgd(boutonMoins,COLOR_PAIR(15));

    box(boutonPlus,0,0);
    box(boutonMoins,0,0);

    wrefresh(boutonPlus);
    wrefresh(boutonMoins);

    /* affichage de la légende */
    wattron(buttonFen,COLOR_PAIR(11));
    mvwaddch(buttonFen,4,1,' ');
    wattroff(buttonFen,COLOR_PAIR(11));
    wmove(buttonFen,4,3);
    wprintw(buttonFen,"Mur invisible");


    wattron(buttonFen,COLOR_PAIR(12));
    mvwaddch(buttonFen,5,1,' ');
    wattroff(buttonFen,COLOR_PAIR(12));
    wmove(buttonFen,5,3);
    wprintw(buttonFen,"Mur visible");


    wattron(buttonFen,COLOR_PAIR(9));
    mvwaddch(buttonFen,6,1,' ');
    wattroff(buttonFen,COLOR_PAIR(9));
    wmove(buttonFen,6,3);
    wprintw(buttonFen,"Guerrier");

    wattron(buttonFen,COLOR_PAIR(1));
    mvwaddch(buttonFen,7,1,'S');
    wattroff(buttonFen,COLOR_PAIR(1));
    wmove(buttonFen,7,3);
    wprintw(buttonFen,"Sortie");

    wmove(buttonFen,8,1);
    wprintw(buttonFen,"Version Carte : %d",getVersion(fdCarte));
    /***********************************/

    wrefresh(buttonFen);
    
    jeuFen = newwin(HAUTEURC+2,LARGEURC+2,11,0);
    box(jeuFen, 0, 0);
    wbkgd(jeuFen,COLOR_PAIR(15));
    wrefresh(jeuFen);

	jeuCarte = createWindowJeu(jeuFen,fdCarte,1,1); /** le 2 ajouté correspond au décalage pour les bordures de la fenètre mère */
    wbkgdset(jeuCarte,COLOR_PAIR(2));
    remplireFenCarte(jeuCarte,fdCarte);
    wrefresh(jeuCarte);


    wbkgd(infosText, COLOR_PAIR(2));
    
    box(infosFen,0,0);
    
    wrefresh(infosText);
    wrefresh(infosFen);
    
    move(LINES-1,0);
    printw("Cliquez dans la fenetre ; pressez F2 pour quitter...");
    while((ch = getch()) != KEY_F(2)) 
        {
        if((ch == KEY_MOUSE) && (souris_getpos(&sourisX, &sourisY, NULL) == OK))
            {
            /* clique sur carte */
            if((sourisX >= 1) && (sourisX <= limitX) && (sourisY >= 12) && (sourisY <= limitY))   {
                wprintw(infosText,"Clic a la position (%d, %d)\n", sourisY, sourisX);
                cliqueCarte(jeuCarte,fdCarte,sourisY-infosFenY-2,sourisX-1);
                }

            /* clique sur boutton pour la vie */
            if((sourisX >= LARGEURC+3) && (sourisX <= (LARGEURC+3)+12) && (sourisY >= 12) && (sourisY <= 14))   {
                if(sourisX >= LARGEURC+3 && sourisX <= (LARGEURC+3)+6) cliqueVie(fdCarte,0);
                else if(sourisX >= (LARGEURC+3)+6 && sourisX <= (LARGEURC+3)+12) cliqueVie(fdCarte,1);
                wmove(buttonFen,0,0); /* on retourne à la première ligne */
                clrtoeol(); /* on supprime la ligne pour afficher le nouveau nbre de vie */
                wprintw(buttonFen,"Vies : %c",getNbVie(fdCarte));
                }
            wmove(buttonFen,8,1); /* on va a la ligne */
            clrtoeol(); /* on supprime la ligne pour afficher la nouvelle version */
            wprintw(buttonFen,"Version Carte : %d",getVersion(fdCarte));
            }
        wrefresh(buttonFen);
        wrefresh(infosText);
        wrefresh(jeuCarte);
        }
    
    /* Suppression de la fenêtre */
        
    delwin(jeuCarte);
    delwin(jeuFen);

    delwin(infosText);
    delwin(infosFen);

    delwin(boutonPlus);
    delwin(boutonMoins);
    delwin(buttonFen);

    /* Fermeture du fichier */
    closeCarte(fdCarte);

    /* Arrêt de ncurses */
    ncurses_stopper();
    
	return EXIT_SUCCESS;
	}
