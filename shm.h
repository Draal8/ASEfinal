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

#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#define SALLE_NAME "/service"
#define ENTRYF_NAME "/entryfile"
#define REGISTRY "/registry"
#define SALLE_SIZE(ntabl) sizeof (struct salle) + (ntabl)*sizeof (struct table)
#define SALLE_UNSIZE(size) size - sizeof (struct salle) / sizeof (struct table)
#define NULLPTR -1
//#define DEBUG_REST 0 le recuperer avec getenv()

#define CHEF_SIZE 10
#define CHECK(op) do { if (op == -1) rerror(#op);} while(0)

noreturn void rerror(char *str);

struct reg_table {
	char chef[CHEF_SIZE];
	char **soumis;
	struct reg_table *suiv;
	
	//tab
};

struct entree {
	char chef[CHEF_SIZE];
	char nom[CHEF_SIZE];
	int nb_convives;
	sem_t client;
	sem_t restaurateur;
};

struct table {
	int nb_places;
	//int places_prises;
	char chef[CHEF_SIZE];	//j'ai glissé, chef
	int suiv;
	sem_t sem;
	sem_t prise;
};

struct salle {
	size_t taille;
	int nb_tables;
	int occupes;
	int libres;
	struct table tables[];
};

void salle_dump (struct salle *sal, FILE * fd);
void *mappy(char *CST_FNAME);
void salle_unmap(void *s);
struct salle *create_tables(int argc, char *argv[]);



#endif

