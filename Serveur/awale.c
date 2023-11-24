#include <stdio.h>

#define NB_PIONS 4
#define NB_CASES 12


void afficherPlateau(int plateau[], int score[]) {
    printf("\nPlateau de jeu :\n");
    printf("Joueur 1 (en bas) : ");
    for (int i = 0; i < NB_CASES / 2; i++) {
        printf("%d ", plateau[i]);
    }
    printf("\nscore: %d ", score[0]);
    printf("\nJoueur 2 (en haut) : ");
    for (int i = NB_CASES - 1; i >= NB_CASES / 2; i--) {
        printf("%d ", plateau[i]);
    }
    printf("\nscore: %d ", score[1]);
    printf("\n");
}

void distribuerPions(int plateau[]) {
    for (int i = 0; i < NB_CASES; i++) {
        plateau[i] = NB_PIONS;
    }
}

int coupValide(int plateau[], int joueur, int position) {
    if ((joueur == 1 && position >= 0 && position < NB_CASES / 2) ||
        (joueur == 2 && position >= NB_CASES / 2 && position < NB_CASES)) {
        if (plateau[position] > 0) {
            return 1;
        }
    }
    return 0;
}

void jouerCoup(int plateau[], int score[], int joueur, int position) {
    int index = position;
    int pions = plateau[position];
    plateau[position] = 0;

    while (pions > 0) {
        index = (index + 1) % NB_CASES;
        if (index == position) {
            continue;
        }
        plateau[index]++;
        pions--;
    }

    while ((joueur == 1 && index >= NB_CASES / 2 && index < NB_CASES && (plateau[index] == 2 || plateau[index] == 3)) || 
        (joueur == 2 && index >= 0 && index < NB_CASES / 2 && (plateau[index] == 2 || plateau[index] == 3))) {
        score[joueur-1] += plateau[index];
        plateau[index] = 0;
        index = (index - 1) % NB_CASES;
    }
}

int finDePartie(int plateau[], int score[]) {
    int totalJoueur1 = 0, totalJoueur2 = 0;

    if(score[0] > NB_CASES*NB_PIONS/2 || score[1] > NB_CASES*NB_PIONS/2) return 1;

    for (int i = 0; i < NB_CASES / 2; i++) {
        totalJoueur1 += plateau[i];
    }
    for (int i = NB_CASES / 2; i < NB_CASES; i++) {
        totalJoueur2 += plateau[i];
    }

    if (totalJoueur1 == 0 || totalJoueur2 == 0) {
        return 1;
    }
    return 0;
}

int main() {
    int plateau[NB_CASES];
    int score[2] = {0,0};
    int joueurActuel = 1;
    int position;

    distribuerPions(plateau);

    while (!finDePartie(plateau, score)) {
        afficherPlateau(plateau, score);

        printf("\nJoueur %d, entrez la position du coup (0-11) : ", joueurActuel);
        scanf("%d", &position);

        if (coupValide(plateau, joueurActuel, position)) {
            jouerCoup(plateau, score, joueurActuel, position);
            joueurActuel = (joueurActuel == 1) ? 2 : 1;
        } else {
            printf("Coup invalide. Veuillez choisir une position valide.\n");
        }
    }

    printf("\nPartie terminée. Score final :\n");
    afficherPlateau(plateau, score);

    return 0;
}