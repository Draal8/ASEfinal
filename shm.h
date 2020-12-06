#ifndef H_SHM
	#define H_SHM
	
	#ifndef _GNU_SOURCE
		#define _GNU_SOURCE	//pour compiler sur ma machine, turing ignorera ceci
	#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdnoreturn.h> //compiler en C11
#include <errno.h>
#include <string.h>

#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#define SALLE_NAME "/service"
#define ENTRYF_NAME "/entryfile"
#define REGISTRY "/registry"
#define SALLE_SIZE(ntabl) sizeof (struct salle) + (ntabl)*sizeof (struct table)
#define SALLE_UNSIZE(size) (size - sizeof (struct salle)) / sizeof (struct table)
#define REGISTRY_SIZE(ntabl) sizeof(struct shm_registre) + (ntabl)*sizeof (struct reg_entr)
#define REGISTRY_UNSIZE(size) (size - sizeof (struct shm_registre)) / sizeof (struct reg_entr)
#define NULLPTR -1
//#define DEBUG_REST 0 le recuperer avec getenv()

#define CHEF_SIZE 11
#define CHECK(op) do { if (op == -1) rerror(#op);} while(0)

noreturn void rerror(char *str);

struct reg_entr {
	int taille;
	char chef[CHEF_SIZE];
	char nom[5][CHEF_SIZE];
};

struct shm_registre {
	int taille;
	sem_t sem;
	struct reg_entr re[];
};

struct reg_table {
	int nb_convives;
	char chef[CHEF_SIZE];
	char **soumis;
	struct reg_table *suiv;
};

struct entree {
	char chef[CHEF_SIZE];
	char nom[CHEF_SIZE];
	int nb_convives;	//sert aussi a renseigner des codes
	sem_t reponse;
	sem_t client;
	sem_t restaurateur;
};

struct table {
	int nb_places;
	int nb_prevus;
	int nb_convives;
	char chef[CHEF_SIZE];	//j'ai glissé, chef
	char nom[5][CHEF_SIZE];
	int suiv;
	sem_t sem;
	sem_t prise;	//sem_timedwait dessus et convive wait dessus
};

struct salle {
	size_t taille;
	int nb_tables;
	int occupes;
	int libres;
	int fermeture;
	struct table tables[];
};

struct th_data {
	int num;
	int delai;
	struct salle *s;
	struct entree *e;
	sem_t sem;
};

void destroy_tables(struct salle *s);
struct entree *create_shm_entree();
void salle_dump (struct salle *sal, FILE * fd);
void *mappy(char *CST_FNAME);
void unmappy(void *v);


#endif

