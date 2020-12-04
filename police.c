#include "shm.h"

int recherche (struct shm_registre *r, struct table t);

//https://www.youtube.com/watch?v=SplJ7U0Jgtw

int main() {
	struct salle *s;
	struct entree *e;
	struct shm_registre *r;
	s = mappy(SALLE_NAME);
	e = mappy(ENTRYF_NAME);
	
	e->nb_convives = -2;
	printf("tables :\n");
	salle_dump(s, stdout);
	sem_wait(&s->police);
	
	sem_post(&e->restaurateur);
	sem_wait(&s->police);
	
	r = mappy(REGISTRY);
	
	int i, j;
	printf("\ncahier d'appel :\n");
	for (i = 0; i < r->taille; i++) {
		printf ("chef : %s\n", r->re[i].chef);
		for (j = 0; j < r->re[i].taille; j++) {
			if (r->re[i].nom[j][0] != '\0')
				printf ("\t%s\n", r->re[i].nom[j]);
		}
	}
	
	sem_post(&s->police);
	unmappy(r);
    return 0;
}

