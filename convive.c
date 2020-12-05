#include "shm.h"

int find_table (struct salle *sal, int nb_c);
int take_table (struct salle *sal, int place, char *chef);
int find_chef (struct salle *sal, char *nom, char *chef);
int registering (struct table t, char *nom, int nb_convives);
void client_seppuku (struct salle *sal, char *nom, char *chef);

int main(int argc, char *argv[]) {
	if (argc < 3) {
		errno = EINVAL;	//replace by usage
		rerror("Not enough arguments (min 3)");
	}
	
	struct salle *s;
	s = mappy(SALLE_NAME);

	salle_dump(s, stdout);
	int i, a = atoi(argv[2]);
	
	if (a == 0) {
		if ((i = find_chef(s, argv[1], argv[2])) == -1) {
			printf("le chef de table n'est pas encore arrive\n");
			//refouler le client
			return -1;
		}
		registering(s->tables[i], argv[1], -1);
	} else {
		if ((i = find_table(s, a)) == -1) {
			printf("aucune table disponible\n");
			//refouler le client
			return -1;	//je m'occupe d'abord du premier convive
		}
		take_table(s, i, argv[1]);
		registering(s->tables[i], argv[1], a);
	}
	salle_dump(s, stdout);
	
	unmappy(s);
	
    return 0;
}


int find_table (struct salle *sal, int nb_c) {
	int i, lasti = -1, tmp, tryw;
	sem_t *s_t;
	
	i = sal->libres;
	while (i != NULLPTR) {
		s_t = &sal->tables[i].sem;
		sem_wait(s_t);
		tmp = sal->tables[i].nb_places;
		tryw = sem_trywait(&sal->tables[i].prise);	//on regarde si la table est prise == 0 alors libre
		//il faut post ensuite et tester EAGAIN
		if (tryw == 0) {
			sem_post(&sal->tables[i].prise);
			if (tmp == nb_c) {
				lasti = i;
				i = NULLPTR;
			} else if (tmp > nb_c && (lasti == -1 || tmp < sal->tables[lasti].nb_places)) {
				lasti = i;
				i = sal->tables[i].suiv;
			} else {
				i = sal->tables[i].suiv;
			}
		} else {
			errno = 0;
			i = sal->tables[i].suiv;
		}
		
		sem_post(s_t);
	}
	
	return lasti;
}


int take_table (struct salle *sal, int place, char *chef) {
	int i, lasti;
	
	sem_wait(&sal->police);
	sem_wait(&sal->tables[place].sem);
	sem_wait(&sal->tables[place].prise);
	
	i = sal->libres;
	if (i == place) {
		sal->libres = sal->tables[place].suiv;
	} else {
		while (sal->tables[i].suiv != place) {
			i = sal->tables[i].suiv;
		}
		sal->tables[i].suiv = sal->tables[place].suiv;
	}
	
	i = sal->occupes;
	if (i == NULLPTR) {
		sal->occupes = place;
	} else {
		lasti = i;
		while (i != NULLPTR) {
			lasti = i;
			i = sal->tables[i].suiv;
		}
		sal->tables[lasti].suiv = place;
	}
	
	sal->tables[place].suiv = NULLPTR;
	strncpy(sal->tables[place].chef, chef, CHEF_SIZE-1);
	sal->tables[place].chef[CHEF_SIZE-1] = '\0';	//au cas ou chef est trop grand
	
	sem_post(&sal->tables[place].sem);
	sem_post(&sal->police);
	return 0;
}

int find_chef(struct salle *sal, char *nom, char *chef) {
	int i, j;
	i = sal->occupes;
	if (i == NULLPTR) {
		return -1;
	} else {
		while (i != NULLPTR) {
			if (strncmp(sal->tables[i].chef, chef, CHEF_SIZE) == 0) {
				for (j = 0; j < sal->tables[i].nb_places-1; j++) {
					if (sal->tables[i].nom[j][0] == '\0') {
						strncpy(sal->tables[i].nom[j], nom, CHEF_SIZE);
						return i;
					}
				}
				
			}
			i = sal->tables[i].suiv;
		}
	}
	return -1;
}


int registering(struct table t, char *nom, int nb_convives) {
	struct entree *e;
	e = mappy(ENTRYF_NAME);
	
	sem_wait(&e->client);
	
	e->nb_convives = nb_convives;
	strncpy(e->chef, t.chef, CHEF_SIZE);	//pas besoin
	strncpy(e->nom, nom, CHEF_SIZE-1);	//besoin de la verif
	e->nom[CHEF_SIZE-1] = '\0';
	
	sem_post(&e->restaurateur);
	unmappy(e);
	return 0;
}

/*
void client_seppuku(struct salle *sal, char *nom, char *chef) {
	int i, j;
	
	for (i = 0; i < sal->nb_tables; i++) {
		if (strncmp(chef, sal->tables[i].chef, CHEF_SIZE) == 0) {
			sem_wait(&sal->tables[i].sem);
			for (j = 0; j < sal->tables[i].nb_places; j++) {
				if (strncmp(nom, sal->tables[i].nom[j], CHEF_SIZE) == 0) {
					memset(sal->tables[i].nom[j], '\0', CHEF_SIZE);
					sem_post(&sal->tables[i].sem);
					return;
				}
			}
			sem_post(&sal->tables[i].sem);
		}
	}
	
}*/

