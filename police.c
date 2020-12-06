#include "shm.h"

void usage();

//https://www.youtube.com/watch?v=SplJ7U0Jgtw

int main(int argc, char *argv[]) {
	if (argc != 0) {
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
	salle_dump(s, stdout);
	//sem_wait(&s->police);
	
	sem_wait(&e->client);//on prends la main au tour du client
	//si c'est libre alors pas de perte de donnees
	//sem_wait(&s->police);
	e->nb_convives = -3;
	sem_post(&e->restaurateur);
	sem_wait(&e->reponse);
	
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
	
	sem_post(&e->client);
	//sem_post(&s->police);
	unmappy(r);
    return 0;
}

void usage() {
	char *str = "usage: ./police\n";
	write(STDERR_FILENO, str, strlen(str));
}

