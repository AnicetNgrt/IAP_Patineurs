//////////////////////////////////////////////////////
//
// fichier: Sprint1.c
// description: Code source du sprint 1 du projet d'IAP 2019
// élèves: Jules et Anicet 
// groupe: 109
//
/////////////////////////////////////////////////////

#pragma warning(disable : 4996)
#include <stdio.h>
#include <stdlib.h>
#define NBR_MAX_ARG 5
#define LONG_LIGNE 91
#define LONG_ARG 31
#define NBR_MAX_EQUIPE 32
#define NBR_MAX_TOUR 10
#define NBR_MAX_EPREUVE 16

FILE* jdt;
FILE* output; 

typedef struct {
	double t;
	int tour;
}Chrono;

typedef Chrono ListeTps[NBR_MAX_TOUR];

typedef struct {
	int compoIndex[3];
	char pays[LONG_ARG];
	double tpsFinal;
}Equipe;

typedef Equipe* EqPt;
typedef Equipe EquipesAdv[2];

typedef struct {
	EquipesAdv equipes;
	int nbrTpsEnrg;
	Equipe* EqGagnante;
}Epreuve;

typedef Epreuve ListeEpr[NBR_MAX_EQUIPE / 2];

typedef struct {
	char nom[LONG_ARG];
	ListeTps temps;
	Equipe* equipe;
	Epreuve* epreuve;
}Patineur;

typedef Patineur ListePat[NBR_MAX_EQUIPE * 3];

typedef struct {
	ListePat patineurs;
	ListeEpr epl;
	int nbrTours;
	int nbrEpreuves;
	int nbrEpreuveTerm;
}Competition;

typedef char arg[LONG_ARG];
typedef arg ListeArg[NBR_MAX_ARG];

void definir_parcours(ListeArg* args, Competition* compet);
void definir_nombre_epreuves(ListeArg* args, Competition* compet);
void inscrire_equipe(ListeArg* args, int index_equipe_epreuve, int nombre_equipes, ListeEpr* epl, ListePat* lpat);
void afficher_equipes(int nombre_epreuve, Competition* compet);
void enregistrer_temps(ListeArg* args, ListePat* patl);
void afficher_temps(ListeArg* args, ListePat* patl);
void detection_fin_poursuite(Competition* compet);
void afficher_temps_final_poursuite(Epreuve* epr, int nbrTours, ListePat* patl);
void detection_fin_competition(Competition* compet);
void classement_final(Competition* compet);

int main(void) {
	jdt = fopen("./datatest.txt", "r");
	output = fopen("./output.txt", "a");

	int nombre_equipes = 0;
	int index_epreuve = 0;
	int index_equipe_epreuve = 0;
	Competition compet;
	compet.nbrEpreuves = 0;
	compet.nbrTours = 0;
	compet.nbrEpreuveTerm = 0;

	while (1) {

		char ligne[LONG_LIGNE];
		ListeArg args;
		int indArg = 0;
		char* extrait;
		int renvoi = fscanf(jdt, "%[^\n]\n", ligne);

		extrait = strtok(ligne, " ");

		while (extrait != NULL && indArg < NBR_MAX_ARG)
		{
			strcpy(args[indArg], extrait);
			extrait = strtok(NULL, " ");
			indArg++;
		}

		if (strcmp(args[0], "definir_parcours") == 0 && compet.nbrTours <= 0) {
			definir_parcours(&args, &compet);
		}

		if (strcmp(args[0], "definir_nombre_epreuves") == 0 && compet.nbrEpreuves <= 0) {
			definir_nombre_epreuves(&args, &compet);
		}

		if (strcmp(args[0], "inscrire_equipe") == 0 && nombre_equipes < compet.nbrEpreuves * 2) {
			if (index_equipe_epreuve > 1) {
				index_epreuve++;
				index_equipe_epreuve = 0;
			}
			inscrire_equipe(&args, index_equipe_epreuve, nombre_equipes, &(compet.epl), &(compet.patineurs));
			nombre_equipes++;
			index_equipe_epreuve += 1;
		}

		if (strcmp(args[0], "afficher_equipes") == 0 && nombre_equipes > 0) {
			afficher_equipes(index_epreuve, &compet);
		}

		if (strcmp(args[0], "enregistrer_temps") == 0 && nombre_equipes > 0) {
			enregistrer_temps(args, &(compet.patineurs));
			detection_fin_poursuite(&compet);
		}

		if (strcmp(args[0], "afficher_temps") == 0 && nombre_equipes > 0) {
			afficher_temps(args, &(compet.patineurs));
		}

		if (strcmp(args[0], "exit") == 0) {
			fclose(jdt);
			exit(0);
		}

	}
	system("pause"); return 0;

}

void definir_parcours(ListeArg* args, Competition* compet) {
	int nbrTr = atoi((*args)[1]);
	compet->nbrTours = nbrTr;
	fprintf(output,">> Format d'epreuve en %d tours selectionne.\n", compet->nbrTours);
	fprintf(output,"   Veuillez renseigner le nombre d'epreuves a planifier.\n\n");
}

void definir_nombre_epreuves(ListeArg* args, Competition* compet) {
	int nbrEp = atoi((*args)[1]);
	compet->nbrEpreuves = nbrEp;
	fprintf(output,">> %d epreuves planifiee(s).\n\n", compet->nbrEpreuves);
}

void inscrire_equipe(ListeArg* args, int index_equipe_epreuve, int nombre_equipes, ListeEpr* epl, ListePat* lpat) {
	int i = index_equipe_epreuve;
	int epi = nombre_equipes / 2;

	if (i == 0) {
		(*epl)[epi].nbrTpsEnrg = -1;
	}

	Patineur pat[3];
	Equipe* equipe = &((*epl)[epi].equipes[i]);
	strcpy((*epl)[epi].equipes[i].pays, (*args)[1]);

	for (int p = 0; p < 3; p++) {
		int pat_index = (nombre_equipes * 3) + p;

		equipe->compoIndex[p] = pat_index;
		strcpy(pat[p].nom, (*args)[p + 2]);
		pat[p].equipe = equipe;
		pat[p].epreuve = &((*epl)[epi]);

		for (int t = 0; t < 3; t++) {
			pat[p].temps[t].t = -1;
			pat[p].temps[t].tour = t + 1;
		}
		(*lpat)[pat_index] = pat[p];

		fprintf(output,">> Patineur inscrit:");
		fprintf(output,"\n  |nom: %s", (*lpat)[pat_index].nom);
		fprintf(output,"\n  |dossard: %d", 101 + pat_index);
		fprintf(output,"\n  |equipe: %s\n\n", (*epl)[epi].equipes[i].pays);
	}
}

void afficher_equipes(int index_epreuve, Competition* compet) {
	fprintf(output,">> Equipes inscrites a l'epreuve:\n");
	// Pour chaque �quipe de l'�preuve
	for (int index_equip = 0; index_equip < 2; index_equip++) {
		Equipe* crtEq = &(compet->epl[index_epreuve].equipes[index_equip]);
		fprintf(output,"\n  ---- %s ----\n", crtEq->pays);

		// Pour chaque patinneur de chaque �preuve
		for (int index_pat = 0; index_pat < 3; index_pat++) {
			Patineur* pat = &(compet->patineurs[(crtEq->compoIndex[index_pat])]);
			int dossard = 101 + (index_epreuve * 6) + (3 * index_equip) + index_pat;
			fprintf(output,"  |--------------\n");
			fprintf(output,"  |nom: %s\n  |dossard: %d\n", pat->nom, dossard);
		}
	}
	fprintf(output,"\n");
}

void enregistrer_temps(ListeArg* args, ListePat* patl){
	int dossard = atoi((*args)[1]);
	int tour = atoi((*args)[2]);

	if (!(dossard >= 101 && dossard <= 101 + (NBR_MAX_EQUIPE * 3))) return;
	if (!(tour > 0 && tour <= NBR_MAX_TOUR)) return;

	double t = atof((*args)[3]);
	Patineur* pat = &((*patl)[dossard - 101]);
	ListeTps* ltps = &(pat->temps);
	Chrono* tps = &((*ltps)[tour]);
	
	if (tps->t == -1) pat->epreuve->nbrTpsEnrg += 1;
	tps->t = t;
	tps->tour = tour;

	fprintf(output,">> Temps enregistre pour %s \n  | dossard %d \n  | tour %d \n  | t: %.2lf sec.\n\n", pat->nom, dossard, tps->tour, tps->t);
}

void afficher_temps(ListeArg* args, ListePat* patl) {
	int dossard = atoi((*args)[1]);
	if (!(dossard >= 101 && dossard <= 101 + (NBR_MAX_EQUIPE * 3))) return;
	Patineur* pat = &(*patl)[dossard - 101];
	ListeTps* ltps = &(pat->temps);

	fprintf(output,">> Temps pour %d | %s | %s :\n", dossard, pat->equipe->pays, pat->nom);

	for (int t = 0; t < 3; t++) {
		Chrono* tps = &((*ltps)[t]);
		if (tps->t != -1) {
			fprintf(output,"  |temps: %.2f sec.\n", tps->t);
			fprintf(output,"  |tour: %d\n\n", tps->tour);
		}
	}
}

void classement_final(Competition* compet) {
	EqPt eqFin[NBR_MAX_EPREUVE];
	Equipe* eqGagnante;
	Equipe* eqPrec;
	Equipe* eqAct;
	int etape = 0;
	

	for (int i = 0; i < compet->nbrEpreuves; i++) {
		eqFin[i] = (compet->epl)[i].EqGagnante;
	}

	while (etape < (compet->nbrEpreuves) - 1) {
		etape++;
		eqAct = eqFin[etape];
		eqPrec = eqFin[etape - 1];

		if (eqAct->tpsFinal <= eqPrec->tpsFinal) {
			eqFin[etape-1] = eqAct;
			eqFin[etape] = eqPrec;
			etape = 0;
		}
	}
	fprintf(output,">>>> FIN COMPETITION !\n");
	fprintf(output,"    |-------------\n    |classement final:\n    |\n");

	for (int i = 0; i < compet->nbrEpreuves; i++) {
		fprintf(output,"    |%d: %s - %0.3f\n",i+1,eqFin[i]->pays,eqFin[i]->tpsFinal);
	}

	fprintf(output,"\n");
	exit(0);
}

void detection_fin_competition(Competition* compet) {
	if (compet->nbrEpreuveTerm == compet->nbrEpreuves) {
		classement_final(compet);
	}
}

void afficher_temps_final_poursuite(Epreuve* epr, int nbrTours, ListePat* patl) {
	double tpsEq1[3];
	double tpsEq2[3];
	char* pays1 = (epr->equipes)[0].pays;
	char* pays2 = (epr->equipes)[1].pays;

	for (int i = 0; i < 3; i++) {
		int index_pat;
		index_pat = (epr->equipes)[0].compoIndex[i];
		tpsEq1[i] = (*patl)[index_pat].temps[2].t;
		index_pat = (epr->equipes)[1].compoIndex[i];
		tpsEq2[i] = (*patl)[index_pat].temps[2].t;
	}
	double temps_final_eq1 = max(max(tpsEq1[0], tpsEq1[1]), tpsEq1[2]);
	double temps_final_eq2 = max(max(tpsEq2[0], tpsEq2[1]), tpsEq2[2]);
	(epr->equipes)[0].tpsFinal = temps_final_eq1;
	(epr->equipes)[1].tpsFinal = temps_final_eq2;

	fprintf(output,">> Fin poursuite:\n");
	fprintf(output,"  |temps:\n");
	fprintf(output,"  |-> %s: %0.3f sec.\n", pays1, temps_final_eq1);
	fprintf(output,"  |-> %s: %0.3f sec.\n  |-----------\n", pays2, temps_final_eq2);

	if (temps_final_eq1 < temps_final_eq2) {
		fprintf(output,"  |%s GAGNE contre %s !\n\n", pays1,pays2);
		epr->EqGagnante = &((epr->equipes)[0]);
	}
	else if (temps_final_eq1 > temps_final_eq2) {
		fprintf(output,"  |%s GAGNE contre %s !\n\n", pays2, pays1);
		epr->EqGagnante = &((epr->equipes)[1]);
	}
	else {
		fprintf(output,"  |EGALITE entre %s et %s !\n\n", pays2, pays1);
		epr->EqGagnante = &((epr->equipes)[0]); // La premi�re �quipe dans la liste gagne en cas d'�galit�...
	}
}

void detection_fin_poursuite(Competition* compet) {
	for (int p = 0; p < compet->nbrEpreuves; p++) {
		Epreuve* epr = &(compet->epl[p]);

		if (epr->nbrTpsEnrg == 6*(compet->nbrTours)-1 && epr->nbrTpsEnrg != -1) {
			afficher_temps_final_poursuite(epr, compet->nbrTours, compet->patineurs);
			epr->nbrTpsEnrg = -1;
			compet->nbrEpreuveTerm +=1;
		}
	}
	detection_fin_competition(compet);
}
