#include "shm.h"

int registering (struct entree *e, char *nom, char *arg2);

int main(int argc, char *argv[]) {
	if (argc < 3) {
		errno = EINVAL;	//replace by usage
		rerror("Not enough arguments (min 3)");
	}
	
	int place;
	struct entree *e;
	struct salle *s;
	e = mappy(ENTRYF_NAME);
	s = mappy(SALLE_NAME);

	salle_dump(s, stdout);
	
	registering(e, argv[1], argv[2]);
	sem_wait(&e->reponse);
	place = e->nb_convives;
	
	if (place == -1) {
		printf("Il n'y a plus de places dans le restaurant\n");
		sem_post(&e->client);
		return -1;
	} else if (place == -2) {
		printf("Votre chef n'est pas encore arrive\n");
		sem_post(&e->client);
		return -2;
	}
	sem_post(&e->client);
	salle_dump(s, stdout);
	//sem_wait(&s->tables[place].prise);
	
	unmappy(s);
	unmappy(e);
	
    return 0;
}


int registering(struct entree *e, char *nom, char *arg2) {
	int a = atoi(arg2);
	sem_wait(&e->client);
	
	e->nb_convives = a;
	if (a == 0) {
		strncpy(e->chef, arg2, CHEF_SIZE);
		strncpy(e->nom, nom, CHEF_SIZE-1);
		e->nom[CHEF_SIZE-1] = '\0';
	} else {
		strncpy(e->chef, nom, CHEF_SIZE-1);
		e->chef[CHEF_SIZE-1] = '\0';
		memset(e->nom, '\0', CHEF_SIZE);
	}
	
	sem_post(&e->restaurateur);
	return 0;
}

