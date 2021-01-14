//////////////////////////////////////////////////////
//
// fichier: Sprint5.c
// description: Code source du sprint 5 du projet d'IAP 2019
// eleves: Jules et Anicet 
// groupe: 109
//
/////////////////////////////////////////////////////

#pragma warning(disable : 4996)
#include <stdio.h>
#include <stdlib.h>
#define NBR_MAX_ARG 5
#define LONG_ARG 31
#define NBR_MAX_EQUIPE 32
#define NBR_MAX_TOUR 10
#define NBR_MAX_EPREUVE 16

FILE* jdt; // Variable globale pour la lecture
FILE* output; // Variable globale pour l'ecriture

/* Nous avons decide de faire beaucoup de definition de types pour rendre 
   les "pointeurs sur tableau de pointeurs" plus simples a manipuler.      */

typedef struct { 
	double t;           // Temps enregistre
	int tour;           // Num. du tour
}Chrono;

// Type servant a stocker tous les temps d'un patineur (d'oe NBR_MAX_TOUR)
typedef Chrono ListeTps[NBR_MAX_TOUR];

typedef struct {
	int compoIndex[3];  // index des patineurs de l'equipe dans la liste des patineurs de la structure "competition"
	char pays[LONG_ARG];// nom du pays (forcement moins long que la longueur max des arguments qui le renseignent
	double tpsFinal;    // temps en fin poursuite (pour un acces plus rapide a la fin de la competition)
}Equipe;

typedef Equipe* EqPt;
typedef Equipe EquipesAdv[2];

typedef struct {
	EquipesAdv equipes; // Tableau des equipes adverses de l'epreuve
	int nbrTpsEnrg;     // Nombre de temps deje enregistres (pour detection_fin_poursuite)
}Epreuve;

typedef Epreuve ListeEpr[NBR_MAX_EQUIPE / 2];

typedef struct {
	char nom[LONG_ARG]; // Nom du patineur
	ListeTps temps;     // Liste des temps du patineur
	EqPt equipe;        // Pointeur vers l'equipe du patineur
	Epreuve* epreuve;   // Pointeur vers l'epreuve du patineur
}Patineur;
// le dossard n'est jamais enregistre car il peut toujours etre (re)calcule dans les fonctions
// qui ont besoin du numero de dossard

typedef Patineur ListePat[NBR_MAX_EQUIPE * 3];

typedef struct {
	ListePat patineurs; // Liste de tous les patineurs dans leur ordre d'inscription
	ListeEpr epl;       // Liste de toutes les epreuves
	int nbrTours;       // Nombre de tours par epreuve
	int nbrEpreuves;    // Nombre d'epreuves
	int nbrEpreuveTerm; // Nombre d'epreuves achevees (pour detection_fin_commpetition)
}Competition;

// Type "argument de commande"
typedef char arg[LONG_ARG];
typedef arg ListeArg[NBR_MAX_ARG];

// La documentation des fonctions se trouve au-dessus de leur code source
void definir_parcours(ListeArg* args, Competition* compet);
void definir_nombre_epreuves(ListeArg* args, Competition* compet);
void inscrire_equipe(ListeArg* args, int index_equipe_epreuve, int nombre_equipes, Competition* compet);
void afficher_equipes(int nombre_epreuve, Competition* compet);
void enregistrer_temps(ListeArg* args, ListePat* patl);
void afficher_temps(ListeArg* args, Competition* compet);
void detection_fin_poursuite(Competition* compet);
void afficher_temps_final_poursuite(Epreuve* epr, Competition* compet);
void detection_fin_competition(Competition* compet);
void classement_final(Competition* compet);

int main(void) {
	jdt = fopen("./datatest.txt", "r"); // fichier de jdt
	output = fopen("./output.txt", "a"); // fichier de sortie

	// initialisation de l'objet "competition" 
	Competition compet;
	compet.nbrEpreuves = 0;
	compet.nbrTours = 0;
	compet.nbrEpreuveTerm = 0;

	// variables servant a l'inscription des equipes 
	// dans les epreuves en fonction de leur ordre d'inscription
	int nombre_equipes = 0; 
	int index_epreuve = 0;
	int index_equipe_epreuve = 0;

	// variables servant a gerer la recuperation des arguments des commandes
	int nombre_arguments_commande = 0;
	int nombre_arguments_collectes = 0;

	// Boucle principale de l'interpreteur de commandes
	while (1) {

		ListeArg args; // liste des arguments (mots) extraits de la ligne
	
		// lecture du fichier de jdt mot par mot
		int renvoi = fscanf(jdt, "%s", &args[nombre_arguments_collectes]);
		
		/*
		---------------------------------
		| TRAITEMENT DES INSTRUCTIONS
		---------------------------------
		*/
		// on ne definit le parcours qu'une seule fois, quand tour > 0 ea veut dire que ea a ete fait
		if (strcmp(args[0], "definir_parcours") == 0 && compet.nbrTours <= 0) {
			nombre_arguments_commande = 1;

			if (nombre_arguments_collectes == nombre_arguments_commande) {
				definir_parcours(&args, &compet);
			}
		}

		// idem pour le nombre d'epreuves
		if (strcmp(args[0], "definir_nombre_epreuves") == 0 && compet.nbrEpreuves <= 0) {
			nombre_arguments_commande = 1;
			
			if (nombre_arguments_collectes == nombre_arguments_commande) {
				definir_nombre_epreuves(&args, &compet);
			}
		}

		// on ne peut pas inscrire plus de 2 equipes par epreuve
		if (strcmp(args[0], "inscrire_equipe") == 0 && nombre_equipes < compet.nbrEpreuves * 2) {
			nombre_arguments_commande = 4;

			if (nombre_arguments_collectes == nombre_arguments_commande) {
				if (index_equipe_epreuve > 1) {
					// passage a l'epreuve suivante si on est au-dele de la 2e equipe definie pour l'epreuve
					index_epreuve++;
					index_equipe_epreuve = 0;
				}                                                     // compet.epl = liste des epreuves
				inscrire_equipe(&args, index_equipe_epreuve, nombre_equipes, &compet);
				// On compte le nombre d'equipe par epreuve et au total pour eviter les depassements
				nombre_equipes++;
				index_equipe_epreuve += 1;
			}	
		}

		if (strcmp(args[0], "afficher_equipes") == 0 && nombre_equipes > 0) {
			afficher_equipes(index_epreuve, &compet);
		}

		if (strcmp(args[0], "enregistrer_temps") == 0 && nombre_equipes > 0) {
			nombre_arguments_commande = 3;

			if (nombre_arguments_collectes == nombre_arguments_commande) {
				enregistrer_temps(&args, &(compet.patineurs));
				
				// a la fin de chaque enregistrement on regarde si la poursuite et/ou la competition sont finies
				detection_fin_poursuite(&compet);
				detection_fin_competition(&compet);
			}
		}

		if (strcmp(args[0], "afficher_temps") == 0 && nombre_equipes > 0) {
			nombre_arguments_commande = 1;
			if (nombre_arguments_collectes == nombre_arguments_commande) {
				afficher_temps(&args, &compet);
			}
		}

		if (strcmp(args[0], "exit") == 0) {
			//sortie du programme
			fclose(jdt);
			fclose(output);
			exit(0);
		}

		if (nombre_arguments_collectes == nombre_arguments_commande) {
			nombre_arguments_collectes = 0;
			nombre_arguments_commande = 0;
		}
		else {
			nombre_arguments_collectes++;
		}
	}
	system("pause"); return 0;
}

/* 
 |void definir_parcours 
 | Permet de definir le nombre de tours par epreuve;
 |----------------
 |Entrees:
 | pointeur sur liste de chaenes: args -> arguments de l'instruction 
 | Competition compet -> pointeur sur l'objet competition de la competition a editer
 |---------------- 
 */
void definir_parcours(ListeArg* args, Competition* compet) {
	int nbrTr = atoi((*args)[1]); // extraction de int depuis char*
	compet->nbrTours = nbrTr;
	fprintf(output,">> Format d'epreuve en %d tours selectionne.\n", compet->nbrTours);
	fprintf(output,"   Veuillez renseigner le nombre d'epreuves a planifier.\n\n");
}

/*
 |void definir_nombre_epreuves 
 | Permet de definir le nombre d'epreuves pour la competition;
 |----------------
 |Entrees:
 | pointeur sur liste de chaenes: args -> arguments de l'instruction
 | Competition* compet -> pointeur sur l'objet competition de la competition a editer
 |----------------
 */
void definir_nombre_epreuves(ListeArg* args, Competition* compet) {
	int nbrEp = atoi((*args)[1]); // extraction de int depuis char*
	compet->nbrEpreuves = nbrEp;
	fprintf(output,">> %d epreuves planifiee(s).\n\n", compet->nbrEpreuves);
}

/*
 |void inscrire_equipe 
 | -> Inscrit les equipes dans une liste d'objet Equipe de taille 2 contenu dans l'objet 
 |    representant l'epreuve oe l'equipe doit etre inscrite.
 | -> Initialise l'epreuve si l'equipe est la premiere inscrite de l'epreuve
 | -> Initialise les 3 patineurs de chaque equipe et leurs temps
 |----------------
 |Entrees:
 | pointeur sur liste de chaenes: args -> arguments de l'instruction
 | Competition* compet -> pointeur sur l'objet competition de la competition dont les sous-objets seront edites 
 |----------------
 */
void inscrire_equipe(ListeArg* args, int index_equipe_epreuve, int nombre_equipes, Competition* compet) {
	int i = index_equipe_epreuve;
	int epi = nombre_equipes / 2; // index de l'epreuve dans la liste des epreuves
	ListeEpr* epl = &(compet->epl); // liste de toutes les epreuves
	ListePat* lpat = &(compet->patineurs); // liste de tous les patineurs
	Epreuve* epAct = &((*epl)[epi]); // pointeur vers l'epreuve editee
	Equipe* equipe = &(epAct->equipes[i]); // pointeur vers l'equipe editee
	Patineur pat[3]; // creation de 3 patineurs qui seront edites puis assignes a leur epreuve

	if (i == 0) {
		// si c'est la premiere equipe de l'epreuve, on initialise l'objet epreuve
		epAct->nbrTpsEnrg = -1;
	}

	strcpy(equipe->pays, (*args)[1]); // attribution du nom du pays renseigne

	// edition des 3 patineurs
	for (int p = 0; p < 3; p++) {
		int pat_index = (nombre_equipes * 3) + p; // index du patineur dans le tableau de tous les patineurs de la competition
		equipe->compoIndex[p] = pat_index;

		strcpy(pat[p].nom, (*args)[p + 2]); // attribution du nom du patineur renseigne
		pat[p].equipe = equipe; // pointeur sur l'equipe du patineur
		pat[p].epreuve = epAct; // pointeur sur l'epreuve du patineur

		// initialisation des temps pour chaque tour
		for (int t = 0; t < compet->nbrTours; t++) {
			pat[p].temps[t].t = -1; // -1 indique que le temps n'est pas enregistre
			pat[p].temps[t].tour = t + 1;
		}
		(*lpat)[pat_index] = pat[p]; // inscription du patineur dans la liste de tous les patineurs

		fprintf(output,">> Patineur inscrit:");
		fprintf(output,"\n  |nom: %s", (*lpat)[pat_index].nom);
		fprintf(output,"\n  |dossard: %d", 101 + pat_index); // le dossard n'est jamais enregistre, mais calcule
		fprintf(output,"\n  |equipe: %s\n\n", epAct->equipes[i].pays);
	}
}

/*
 |void afficher equipes
 | Permet d'afficher les deux equipes de l'epreuve en cours
 |----------------
 |Entrees:
 | int index_epreuve -> index de l'epreuve en cours dans la liste des epreuves de l'objet competition
 | Competition* compet -> pointeur sur l'objet competition de la competition
 |----------------
 */
void afficher_equipes(int index_epreuve, Competition* compet) {
	fprintf(output,">> Equipes inscrites a l'epreuve:\n");
	Epreuve* epAct = &(compet->epl[index_epreuve]); // pointeur sur l'epreuve traitee

	// Iteration a-travers la liste des equipes de l'epreuve
	for (int index_equip = 0; index_equip < 2; index_equip++) {

		Equipe* crtEq = &(epAct->equipes[index_equip]); // pointeur sur l'equipe traitee

		fprintf(output,"\n  ---- %s ----\n", crtEq->pays); // affichage du pays

		// Iteration a-travers la liste des patineurs de l'equipe
		for (int index_pat = 0; index_pat < 3; index_pat++) {
			// int index_pat: index de l'index global du patineur dans la liste des index globaux des 3 patineurs de l'equipe
			int index_global = crtEq->compoIndex[index_pat]; // index du patineur dans la liste de tous les patineurs
			Patineur* pat = &(compet->patineurs[index_global]); // pointeur sur le patineur traite

			int dossard = 101 + (index_epreuve * 6) + (3 * index_equip) + index_pat; // dossard recalcule
			fprintf(output,"  |--------------\n");
			fprintf(output,"  |nom: %s\n  |dossard: %d\n", pat->nom, dossard); // affichage du nom et du dossard
		}
	}
	fprintf(output,"\n");
}
/*
 |void enregistrer_temps
 | Permet d'enregistrer un temps pour un patineur donne et pour un tour donne
 |----------------
 |Entrees:
 | int index_epreuve -> index de l'epreuve en cours dans la liste des epreuves de l'objet competition
 | pointeur sur liste de patineurs: patl -> liste de tous les patineurs indexe selon le numero du dossard -101
 |----------------
 */
void enregistrer_temps(ListeArg* args, ListePat* patl){
	int dossard = atoi((*args)[1]); // numero du dossard int extrait d'un argument char*
	int tour = atoi((*args)[2]); // numero du tour int extrait d'un argument char*

	if (!(dossard >= 101 && dossard <= 101 + (NBR_MAX_EQUIPE * 3))) return; // numero de dossard invalide
	if (!(tour > 0 && tour <= NBR_MAX_TOUR)) return; // numero du tour invalide

	double t = atof((*args)[3]); // temps de type float extrait d'un argument char*
	Patineur* pat = &((*patl)[dossard - 101]); // pointeur sur le patineur
	ListeTps* ltps = &(pat->temps); // pointeur sur la liste des temps du patineur
	Chrono* tps = &((*ltps)[tour-1]); // pointeur sur le temps concerne qui va etre edite
	
	// si c'est la 1ere fois qu'on enregistre ce temps, on increment le nombre de temps enregistres pour l'epreuve du patineur
	if (tps->t == -1) pat->epreuve->nbrTpsEnrg += 1;

	// edition du temps concerne
	tps->t = t;
	tps->tour = tour;

	fprintf(output,">> Temps enregistre pour %s \n  | dossard %d \n  | tour %d \n  | t: %.2lf sec.\n\n", pat->nom, dossard, tps->tour, tps->t);
}

/*
 |void afficher_temps
 | Permet d'afficher tous les temps enregistres pour un patineur donne
 |----------------
 |Entrees:
 | int index_epreuve -> index de l'epreuve en cours dans la liste des epreuves de l'objet competition
 | Competition* compet -> pointeur sur l'objet competition de la competition
 |----------------
 */
void afficher_temps(ListeArg* args, Competition* compet) {
	int dossard = atoi((*args)[1]); // extraction du numero de dossard int depuis l'argument char*
	if (!(dossard >= 101 && dossard <= 101 + (NBR_MAX_EQUIPE * 3))) return; // numero de dossard invalide

	ListePat* patl = &(compet->patineurs); // pointeur sur liste des patineurs
	Patineur* pat = &(*patl)[dossard - 101]; // pointeur sur le patineur concerne
	ListeTps* ltps = &(pat->temps); // pointeur sur la liste des temps du patineur

	// affichage de l'entete
	fprintf(output,">> Temps pour %d | %s | %s :\n", dossard, pat->equipe->pays, pat->nom);

	// pour chaque temps du patineur
	for (int t = 0; t < compet->nbrTours; t++) {
		Chrono* tps = &((*ltps)[t]); // objet temps traite
		// verifie que le temps est enregistre (la valeur par defaut du temps.t est -1)
		if (tps->t != -1) {
			fprintf(output,"  |temps: %.2f sec.\n", tps->t);
			fprintf(output,"  |tour: %d\n\n", tps->tour);
		}
	}
}

/*
 |void classement_final
 | Permet de trier les equipes par temps a la fin de la competition et d'afficher
 | le classement final avec les temps dans l'ordre decroissant
 |----------------
 |Entrees:
 | Competition* compet -> pointeur sur l'objet competition de la competition
 |----------------
 */
void classement_final(Competition* compet) {
	EqPt liste_equipes[NBR_MAX_EPREUVE*2]; // liste d'equipe qui va etre remplie puis triee
	Equipe* eqPrec; // pointeur sur l'equipe precedente dans liste_equipes
	Equipe* eqAct; // pointeur sur l'equipe en cours de traitement dans liste_equipes
	int etape = 0; // etape dans la liste
	
	// creation d'une liste avec toutes les equipes
	// On itere a-travers la liste des equipes a remplir ainsi qu'a travers la liste des epreuves
	for (int i = 0; i < (compet->nbrEpreuves * 2); i += 2) {
		Epreuve* epAct = &((compet->epl)[i / 2]); // epreuve traitee
		liste_equipes[i] = &(epAct->equipes[0]); // equipe ne1 de l'epreuve
		liste_equipes[i + 1] = &(epAct->equipes[1]); // equipe ne2 de l'epreuve
	}

	// ALGORITHME DE TRI DANS L'ORDRE CROISSANT:
	//------------------------------------------------------------------------------------------
	// On parcours la liste jusqu'a temps que la valeur precedente soit superieur a la valeur
	// actuelle. Lorsque c'est le cas, l'on intervertit les deux valeurs et l'on revient a la 2e
	// valeur de la liste. Lorsque l'on arrive enfin au bout de la liste (i = longueurListe - 1)
	// cela implique que tous les couples de valeurs desordonnees ont ete ordonnes, donc que la 
	// liste est triee dans l'ordre croissant.
	// Ici on ne compare pas les variables "Equipe" en elles-memes, mais l'attribut "temps final" des equipes.

	while (etape < (compet->nbrEpreuves*2) - 1) {
		etape++;
		eqAct = liste_equipes[etape];
		eqPrec = liste_equipes[etape - 1];

		if (eqAct->tpsFinal <= eqPrec->tpsFinal) {
			liste_equipes[etape-1] = eqAct;
			liste_equipes[etape] = eqPrec;
			etape = 0;
		}
	}

	// affichage de l'entete
	fprintf(output,">>>> FIN COMPETITION !\n");
	fprintf(output,"    |-------------\n    |classement final:\n    |-------------\n");

	// on itere a-travers la liste ordonnee des equipes et on affiche leur temps final;
	for (int i = 0; i < compet->nbrEpreuves*2; i++) {
		fprintf(output,"    |%d: %s - %0.3f sec.\n",i+1,liste_equipes[i]->pays,liste_equipes[i]->tpsFinal);
	}
	fprintf(output,"\n");
	
	// sortie du programme
	fclose(jdt);
	fclose(output);
	exit(0);
}

/*
 |void detection_fin_competition
 | Verifie si la competition est finie, si oui alors elle renvoie vers la fonction classement_final
 |----------------
 |Entrees:
 | Competition* compet -> pointeur sur l'objet competition de la competition
 |----------------
 */
void detection_fin_competition(Competition* compet) {
	// on regarde si toutes les epreuves sont terminees
	if (compet->nbrEpreuveTerm == compet->nbrEpreuves) {
		classement_final(compet);
	}
}

/*
 |void afficher_temps_final_poursuite
 | ->Permet d'afficher les temps finaux d'une poursuite, l'equipe gagnante et perdante, 
 |   ou l'egalite entre deux equipes. 
 | ->Permet de mettre a jour des variables cles a la detection de fin de competition comme
 |   le nombre de poursuites terminees, et les temps finaux de chaque equipe.
 |----------------
 |Entrees:
 | Epreuve* epr -> pointeur sur l'epreuve terminee
 | Competition* compet -> pointeur sur l'objet competition de la competition
 |----------------
 */
void afficher_temps_final_poursuite(Epreuve* epr, Competition* compet) {
	ListePat* patl = &(compet->patineurs); // pointeur sur la liste de tous les patineurs
	int nbrTours = compet->nbrTours; // nombre de tours par epreuve
	double tpsEq1[3]; // tableau des derniers temps des patineurs de l'equipe ne1
	double tpsEq2[3]; // tableau des derniers temps des patineurs de l'equipe ne2
	char* pays1 = (epr->equipes)[0].pays; // nom du pays de l'equipe ne1
	char* pays2 = (epr->equipes)[1].pays; // nom du pays de l'equipe ne2

	// Pour chaque patineur, on copie le temps au dernier tour dans le tabeau des temps de l'equipe.
	for (int i = 0; i < 3; i++) {
		int index_pat; // index des 2 patineurs traites dans la liste de tous les patineurs

		index_pat = (epr->equipes)[0].compoIndex[i]; // index du patineur i de l'equipe 1
		tpsEq1[i] = (*patl)[index_pat].temps[(compet->nbrTours) - 1].t; // temps final du patineur i de l'equipe 1

		index_pat = (epr->equipes)[1].compoIndex[i]; // index du patineur i de l'equipe 2
		tpsEq2[i] = (*patl)[index_pat].temps[(compet->nbrTours) - 1].t; // temps final du patineur i de l'equipe 1
	}
	double temps_final_eq1 = max(max(tpsEq1[0], tpsEq1[1]), tpsEq1[2]); // le plus grand des 3 temps finaux de l'equipe 1
	double temps_final_eq2 = max(max(tpsEq2[0], tpsEq2[1]), tpsEq2[2]); // le plus grande des 3 temps finaux de l'equipe 2
	
	// on stocke le plus grand temps final des equipes dans leur propre  
	// objet Equipe, pour un acces simplifie a la fin de la competition
	(epr->equipes)[0].tpsFinal = temps_final_eq1;
	(epr->equipes)[1].tpsFinal = temps_final_eq2;

	// affichage des temps finaux
	fprintf(output,">> Fin poursuite:\n");
	fprintf(output,"  |temps:\n");
	fprintf(output,"  |-> %s: %0.3f sec.\n", pays1, temps_final_eq1);
	fprintf(output,"  |-> %s: %0.3f sec.\n  |-----------\n", pays2, temps_final_eq2);

	// affichage du gagnant et du perdant ou de l'egalite
	if (temps_final_eq1 < temps_final_eq2) {
		fprintf(output,"  |%s GAGNE contre %s !\n\n", pays1,pays2);
	}
	else if (temps_final_eq1 > temps_final_eq2) {
		fprintf(output,"  |%s GAGNE contre %s !\n\n", pays2, pays1);
	}
	else {
		fprintf(output,"  |EGALITE entre %s et %s !\n\n", pays2, pays1);
	}
}

/*
 |void detection_fin_poursuite
 | Permet de detecter la fin d'une poursuite, et de renvoyer vers la fonction d'affichage
 | de fin de poursuite dans le cas echeant
 |----------------
 |Entrees:
 | Competition* compet -> pointeur sur l'objet competition de la competition
 |----------------
 */
void detection_fin_poursuite(Competition* compet) {
	// Iteration a-travers la liste des epreuves
	for (int p = 0; p < compet->nbrEpreuves; p++) {
		Epreuve* epr = &(compet->epl[p]); // epreuve a verifier

		// une epreuve est terminee si le nombre de temps enregistres au moins une fois
		// est egal au nombre de tour multiplie par le nombre de patineur
		// ici on fait -1 car compet->nbrTours commence a 0
		if (epr->nbrTpsEnrg == 6*(compet->nbrTours)-1 && epr->nbrTpsEnrg != -1) {
			afficher_temps_final_poursuite(epr, compet);
			epr->nbrTpsEnrg = -1; // permet d'empecher l'affichage de cette epreuve si on rerentre dans detection_fin_poursuite a nouveau
			compet->nbrEpreuveTerm +=1; // compteur du nombre d'epreuves terminees pour detection_fin_competition
		}
	}
}
