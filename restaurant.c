#include "shm.h"

int arg_check(int argc, char *argv[]);
int find_chef(struct salle *sal, char *nom, char *chef);
int find_table (struct salle *sal, int nb_c);
int take_table (struct salle *sal, int place, char *chef);
void add_entree(struct salle *s, struct entree *e);
void add_soumis(struct salle *s, struct entree *e, int place);
void recopier_registre();
struct salle *create_tables(struct entree *e, int argc, char *argv[]);
void thr_tabl(struct entree *e, struct salle *s, int delai, int nb_tables);
void *f (void *arg);
void usage();

struct reg_table *registre = NULL;
int reg_taille = 0;

pthread_t *pth;
struct th_data *th;

int main(int argc, char *argv[]) {
	if (argc < 3) {
		usage();
		return -1;
	}
	
	if (arg_check(argc, argv) == 1) {
		usage();
		return -1;
	}
	
	int wt, place;
	struct salle *s;
	struct entree *e;
	e = create_shm_entree();
	s = create_tables(e, argc, argv);
	
	salle_dump(s, stdout);
	wt = sem_wait(&e->restaurateur);
	
	while (wt == 0) {
		if (e->nb_convives == -3) {
			recopier_registre(s);
			sem_post(&e->reponse);
		} else if (e->nb_convives == 0) {
			place = find_chef(s, e->nom, e->chef);
			if (place != -1) {
				add_soumis(s, e, place);	//et oui il faut penser au feminin comme au masculin
				e->nb_convives = place;
			} else {
				e->nb_convives = -2;
			}
			sem_post(&e->reponse);
		} else {
			place = find_table(s, e->nb_convives);
			if (place != -1) {
				take_table(s, place, e->chef);
				add_entree(s, e);
				e->nb_convives = place;
				if (s->tables[place].nb_prevus == 1)
					sem_post(&th[place].sem);
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
	
	sal->tables[lasti].nb_prevus = nb_c;
	return lasti;
}


int take_table (struct salle *sal, int place, char *chef) {
	int i, lasti;
	
	sem_wait(&sal->tables[place].sem);
	//sem_wait(&sal->tables[place].prise);
	
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
	
	sal->tables[place].nb_convives = 1;
	
	sem_post(&sal->tables[place].sem);
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


void add_soumis(struct salle *s, struct entree *e, int place) {
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
	
	s->tables[place].nb_convives++;
	if (s->tables[place].nb_convives == tmp->nb_convives) {
		sem_post(&th[place].sem);
	}
}


void recopier_registre() {
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
}


struct salle *create_tables(struct entree *e, int argc, char *argv[]) {
	int fd, i;
	struct salle *s;
	CHECK((fd = shm_open(SALLE_NAME, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU)));
	CHECK(ftruncate(fd, (off_t) SALLE_SIZE(argc-2)));
	if ((s = mmap(NULL, SALLE_SIZE(argc-2), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == NULL) rerror("mmap create_tables()");
	
	s->taille = SALLE_SIZE(argc-2);
	s->occupes = NULLPTR;
	s->libres = 0;
	s->nb_tables = argc-2;
	
	for (i = 0; i < argc-2; i++) {
		s->tables[i].suiv = i+1;
		s->tables[i].nb_places = atoi(argv[i+2]);
		if (s->tables[i].nb_places == 0) {
			rerror("Un argument n'est pas un chiffre > 0");
		}
		s->tables[i].chef[0] = '\0';
		s->tables[i].nb_prevus = 0;
		s->tables[i].nb_convives = 0;
		sem_init(&s->tables[i].sem, 1, 1);
		sem_init(&s->tables[i].prise, 1, 0);
	}
	s->tables[i-1].suiv = NULLPTR;	//on modifie le dernier pour lui mettre la bonne valeur
	
	thr_tabl(e, s, atoi(argv[1]), argc-2);
	return s;
}


void thr_tabl(struct entree *e, struct salle *s, int delai, int nb_tables) {
	pthread_t *tid;
	int i;
	struct th_data *ti;
	tid = malloc (nb_tables * sizeof(pthread_t));
	ti = malloc (nb_tables * sizeof(struct th_data));
	
	pth = tid;
	th = ti;
	
	for (i = 0; i < nb_tables; i++) {
		ti[i].num = i;
		ti[i].delai = delai;
		ti[i].s = s;
		ti[i].e = e;
		sem_init(&ti[i].sem, 1, 0);
		if ((errno = pthread_create(&tid[i], NULL, f, &ti[i])) != 0)
			rerror("pthread_create");
		//printf ("creation de %ld\n", tid[i]);
	}
}


void *f (void *arg) {
	int i;
	struct th_data *data = arg;
	struct timespec t;
	long int nsec = (data->delai%1000) * 1000000;
	long int sec = data->delai/1000;
	
	//boucle
	//printf("on attends les clients\n");
	sem_wait(&data->sem);	//signal de prise de la table
	//printf("table prise\n");
	
	while (1) {
		clock_gettime(CLOCK_REALTIME, &t);
		t.tv_sec += sec + (t.tv_nsec + nsec)/1000000000;
		t.tv_nsec = (t.tv_nsec + nsec)%1000000000;
		sem_timedwait(&data->s->tables[data->num].prise, &t);
		sem_wait(&data->e->client);	//on bloque l'entree
		sem_wait(&data->s->tables[data->num].sem);	//on bloque la table
		
		errno = 0;
		for (i = 0; i < data->s->tables[data->num].nb_convives ; i++) {
			sem_post(&data->s->tables[data->num].prise);
			printf ("data->num = %d\n", data->num);
			//les clients peuvent se terminer
		}
		
		memset(data->s->tables[data->num].chef, '\0', CHEF_SIZE);
		for (i = 0; i < data->s->tables[data->num].nb_places; i++) {
			memset(data->s->tables[data->num].nom[i], '\0', CHEF_SIZE);
		}
		
		data->s->tables[data->num].nb_prevus = 0;
		data->s->tables[data->num].nb_convives = 0;
		
		salle_dump(data->s, stdout);
		sem_post(&data->s->tables[data->num].sem);
		sem_post(&data->e->client);
	}
	
	pthread_exit(0);
}

void usage() {
	char *str = "usage: ./restaurant tmp(ms) nb1 nb2 ...\n";
	write(STDERR_FILENO, str, strlen(str));
}

int arg_check(int argc, char *argv[]) {
	int i, j, a;
	for (i = 1; i < argc; i++) {
		a = atoi(argv[i]);
		if (a < 1 || a > 6) return 1;
		for (j = 0; j < (int) strlen(argv[i]); j++) {
			if (argv[i][j] < 48 || argv[i][j] > 57) return 1;
		}
	}
	return 0;
}

