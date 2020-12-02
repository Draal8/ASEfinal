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
            int v;
            fprintf (fd, " %s", tabl->chef);
            
            if (sem_getvalue (&(tabl->prise), &v) == -1)
                perror ("sem_getvalue");
            else
                fprintf (fd, " (%d)", v);
        }
        
        fprintf (fd, "\n");
    }
}

void *mappy(char *CST_FNAME) {
	int fd;
	struct stat s;
	struct salle *sal;
	
	CHECK((fd = shm_open(CST_FNAME, O_RDWR, S_IRWXU)));
	CHECK(fstat(fd, &s));
	if ((sal = mmap(NULL, s.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == NULL) rerror("mmap stock_create()");
	
	return sal;
}

void salle_unmap(void *s) {
	munmap(s, sizeof(*s));
}




