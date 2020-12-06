#include "shm.h"

void usage();

//https://www.youtube.com/watch?v=SplJ7U0Jgtw

int main(int argc, char *argv[]) {
	if (argc != 1) {
		if (argv[0][0] == '\0') {;}	//un moment on nous force a compiler avec Werror
		usage();
		return -1;
	}
	
	struct salle *s;
	struct entree *e;
	struct shm_registre *r;
	s = mappy(SALLE_NAME);
	e = mappy(ENTRYF_NAME);
	
	e->nb_convives = -1;
	printf("tables :\n");
	//salle_dump(s, stdout);
	//sem_wait(&s->police);
	
	sem_wait(&e->client);//on prends la main au tour du client
	//si c'est libre alors pas de perte de donnees
	//sem_wait(&s->police);
	e->nb_convives = -3;
	sem_post(&e->restaurateur);
	sem_wait(&e->police);
	sem_post(&e->client);
	
	r = mappy(REGISTRY);
	
	int i, j;
	for (i = 0; i < s->nb_tables; i++) {
		if (s->tables[i].nb_convives != 0) {
			printf("Table %d : %s ", i, s->tables[i].chef);
			for (j = 0; j < s->tables[i].nb_convives-1; j++) {
				printf("%s ", s->tables[i].nom[j]);
			}
			printf("\n");
		} else {
			printf("Table %d : (vide)\n", i);
		}
	}
	
	printf("\ncahier de rappels :\n");
	for (i = 0; i < r->taille; i++) {
		printf ("Groupe %d : %s ", i+1, r->re[i].chef);
		for (j = 0; j < r->re[i].taille; j++) {
			if (r->re[i].nom[j][0] != '\0')
				printf ("%s ", r->re[i].nom[j]);
		}
		printf("\n");
	}
	
	//sem_post(&s->police);
	sem_destroy(&r->sem);
	CHECK(shm_unlink(REGISTRY));
	
    return 0;
}

void usage() {
	char *str = "usage: ./police\n";
	write(STDERR_FILENO, str, strlen(str));
}

