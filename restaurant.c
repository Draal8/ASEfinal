#include "shm.h"

struct salle *create_tables(int argc, char *argv[]);
struct entree *create_shm_entree();
void destroy_tables(struct salle *s);
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
	
	int wt;
	struct salle *s;
	struct entree *e;
	
	s = create_tables(argc, argv);
	e = create_shm_entree();
	
	salle_dump(s, stdout);
	
	wt = sem_wait(&e->restaurateur);
	
	while (wt == 0) {
		if (e->nb_convives == -2) {
			recopier_registre(s);
		} else {
			if (strncmp(e->chef, e->nom, CHEF_SIZE) == 0) {
				add_entree(s, e);
			} else {
				add_soumis(e);	//et oui il faut penser au feminin comme au masculin
			}
		}
		wt = sem_wait(&e->restaurateur);
	}
	
    return 0;
}


struct salle *create_tables(int argc, char *argv[]) {
	int fd, i;
	struct salle *s;
	CHECK((fd = shm_open(SALLE_NAME, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU)));
	CHECK(ftruncate(fd, (off_t) SALLE_SIZE(argc-2)));
	if ((s = mmap(NULL, SALLE_SIZE(argc-2), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == NULL) rerror("mmap create_tables()");
	
	s->taille = SALLE_SIZE(argc-2);
	s->occupes = NULLPTR;
	s->libres = 0;
	
	for (i = 0; i < argc-2; i++) {
		s->tables[i].suiv = i+1;
		s->tables[i].nb_places = atoi(argv[i+2]);
		if (s->tables[i].nb_places == 0) {
			rerror("Un argument n'est pas un chiffre > 0");
		}
		s->tables[i].chef[0] = '\0';
		sem_init(&s->tables[i].sem, 1, 1);
		sem_init(&s->tables[i].prise, 1, 1);
	}
	s->tables[i-1].suiv = NULLPTR;	//on modifie le dernier pour lui mettre la bonne valeur
	s->nb_tables = 0;
	
	sem_init(&s->police, 1, 1);
	return s;
}


struct entree *create_shm_entree() {
	int fd;
	struct entree *e;
	
	CHECK((fd = shm_open(ENTRYF_NAME, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU)));
	CHECK(ftruncate(fd, sizeof(struct entree)));
	if ((e = mmap(NULL, sizeof(struct entree), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == NULL) rerror("mmap create_tables()");
	
	e->chef[0] = '\0';
	e->nom[0] = '\0';
	
	sem_init(&e->client, 1, 1);
	sem_init(&e->restaurateur, 1, 0);
	
	return e;
}


void destroy_tables(struct salle *s) {
	int fd;
	long unsigned int i;
	
	CHECK((fd = shm_open(SALLE_NAME, O_RDWR, S_IRWXU)));
	
	for (i = 0; i < SALLE_UNSIZE(sizeof(s)); i++) {
		sem_destroy(&s->tables[i].sem);
	}
    CHECK(shm_unlink(SALLE_NAME));
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
		rt = rt->suiv;
	}
	
	sem_init(&reg->sem, 1, 1);
	unmappy(reg);
	sem_post(&sal->police);
}


