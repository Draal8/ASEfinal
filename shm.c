#include "shm.h"

noreturn void rerror(char *str) {
	if (errno != 0) {
		perror(str);
	} else {
		write(STDOUT_FILENO, str, strlen(str));
		write(STDOUT_FILENO, "\n", 1);
	}
    exit(EXIT_FAILURE);
}

//struct salle *create_tables(int argc, char *argv[]);

struct entree *create_shm_entree() {
	int fd;
	struct entree *e;
	
	CHECK((fd = shm_open(ENTRYF_NAME, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU)));
	CHECK(ftruncate(fd, sizeof(struct entree)));
	if ((e = mmap(NULL, sizeof(struct entree), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == NULL) rerror("mmap create_tables()");
	
	e->chef[0] = '\0';
	e->nom[0] = '\0';
	
	sem_init(&e->reponse, 1, 0);
	sem_init(&e->client, 1, 1);
	sem_init(&e->restaurateur, 1, 0);
	sem_init(&e->police, 1, 0);
	
	return e;
}

void salle_dump (struct salle *sal, FILE * fd) {
    fprintf (fd, "ADRESSE DU SEGMENT %p\n", sal);
    fprintf (fd, "LIBRES %d OCCUPES %d", sal->libres, sal->occupes);
    size_t n =
        (sal->taille - sizeof (struct salle)) / sizeof (struct table);
    fprintf (fd, " (|table|=%zub |salle|=%zub size=%zub -> %zu tables)\n",
             sizeof (struct table), sizeof (struct salle),
             sal->taille, n);
             
    for (size_t i=0 ; i<n ; i++) {
        struct table *tabl = &(sal->tables[i]);
        fprintf (fd, "%4zd (->%4d): (%dp)", i, tabl->suiv, tabl->nb_places);
        
        if (tabl->chef[0] != '\0') {
            int v, j;
            fprintf (fd, " %s", tabl->chef);
            
            if (sem_getvalue (&(tabl->prise), &v) == -1)
                perror ("sem_getvalue");
            else
                fprintf (fd, " (%d)", v);
            
            for (j = 0; j < tabl->nb_places; j++) {
            	if (tabl->nom[j][0] != '\0') {
            		printf (" %s", tabl->nom[j]);
            	}
            }
        }
        
        fprintf (fd, "\n");
    }
}

void *mappy(char *CST_FNAME) {
	int fd;
	struct stat s;
	void *v;
	
	CHECK((fd = shm_open(CST_FNAME, O_RDWR, S_IRWXU)));
	CHECK(fstat(fd, &s));
	if ((v = mmap(NULL, s.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == NULL) rerror("mmap stock_create()");
	
	return v;
}

void unmappy(void *v) {
	munmap(v, sizeof(*v));
}




