#include "shm.h"

int find_chef(struct salle *sal, char *nom, char *chef);
int find_table (struct salle *sal, int nb_c);
int take_table (struct salle *sal, int place, char *chef);
void add_entree(struct salle *s, struct entree *e);
void add_soumis(struct entree *e);
void recopier_registre(struct salle *sal);

struct reg_table *registre = NULL;
int reg_taille = 0;

int main(int argc, char *argv[]) {
	if (argc < 3) {
		errno = EINVAL;	//replace by usage
		rerror("Not enough arguments (min 3)");
	}
	
	int wt, place;
	struct salle *s;
	struct entree *e;
	s = create_tables(argc, argv);
	e = create_shm_entree();
	
	salle_dump(s, stdout);
	wt = sem_wait(&e->restaurateur);
	
	while (wt == 0) {
		if (e->nb_convives == -3) {
			recopier_registre(s);
			sem_post(&e->reponse);
		} else if (e->nb_convives == 0) {
			place = find_chef(s, e->nom, e->chef);
			if (place != -1) {
				add_soumis(e);	//et oui il faut penser au feminin comme au masculin
			} else {
				e->nb_convives = -2;
			}
			sem_post(&e->reponse);
		} else {
			place = find_table(s, e->nb_convives);
			if (place != -1) {
				take_table(s, place, e->chef);
				add_entree(s, e);
			} else {
				e->nb_convives = -1;
			}
			sem_post(&e->reponse);
		}
		
		wt = sem_wait(&e->restaurateur);
	}
	
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


int find_table (struct salle *sal, int nb_c) {
	int i, lasti = -1, tmp;
	sem_t *s_t;
	
	i = sal->libres;
	while (i != NULLPTR) {
		s_t = &sal->tables[i].sem;
		sem_wait(s_t);
		tmp = sal->tables[i].nb_places;
		
		if (sal->tables[i].chef[0] == '\0') {	//donc libre
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
			i = sal->tables[i].suiv;
		}
		
		
		sem_post(s_t);
	}
	
	return lasti;
}


int take_table (struct salle *sal, int place, char *chef) {
	int i, lasti;
	
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


void add_entree(struct salle *s, struct entree *e) {
	int i;
	struct reg_table *r, *tmp;
	if ((r = malloc(sizeof(*r))) == NULL) {
		rerror("malloc");
	}
	
	if (e->nb_convives != 1) {	//on a deja le chef
		if ((r->soumis = malloc((e->nb_convives-1) * sizeof(char *))) == NULL) rerror("malloc");
		for (i = 0; i < e->nb_convives-1; i++) {
			if ((r->soumis[i] = malloc(CHEF_SIZE)) == NULL) rerror("malloc");
			r->soumis[i][0] = '\0';
		}
	} else {
		r->soumis = NULL;
	}
	
	strncpy(r->chef, e->chef, CHEF_SIZE);
	r->nb_convives = e->nb_convives;
	
	if (registre == NULL) {
		registre = r;
		registre->suiv = NULL;
	} else {
		tmp = registre;
		while (tmp->suiv != NULL) {
			tmp = tmp->suiv;
		}
		tmp->suiv = r;
	}
	reg_taille++;
	s->nb_tables++;
	sem_post(&e->client);
}


void add_soumis(struct entree *e) {
	int i;
	struct reg_table *tmp;
	tmp = registre;
	while (strncmp(tmp->chef, e->chef, CHEF_SIZE) != 0) {
		tmp = tmp->suiv;
	}
	for (i = 0; i < tmp->nb_convives-1 && tmp->soumis[i][0] != '\0'; i++);
	if (i < tmp->nb_convives-1) {
		strncpy(tmp->soumis[i], e->nom, CHEF_SIZE);
	}
		
	sem_post(&e->client);
}


void recopier_registre(struct salle *sal) {
	int fd, i, j;
	struct reg_table *rt = registre;
	struct shm_registre *reg;
	
	CHECK((fd = shm_open(REGISTRY, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU)));
	CHECK(ftruncate(fd, REGISTRY_SIZE(reg_taille)));
	if ((reg = mmap(NULL, REGISTRY_SIZE(reg_taille), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == NULL) rerror("mmap create_tables()");
	
	reg->taille = reg_taille;
	
	for (i = 0; i < reg_taille; i++) {
		strncpy(reg->re[i].chef, rt->chef, CHEF_SIZE);
		reg->re[i].taille = rt->nb_convives; // la taille est remise a 0 pour une raison inconnue
		
		for (j = 0; j < rt->nb_convives-1; j++) {
			strncpy(reg->re[i].nom[j], rt->soumis[j], CHEF_SIZE);
		}
		for (j = rt->nb_convives-1; j < 5; j++) {
			reg->re[i].nom[j][0] = '\0';
		}
		rt = rt->suiv;
	}
	
	sem_init(&reg->sem, 1, 1);
	unmappy(reg);
	sem_post(&sal->police);
}


