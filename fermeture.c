#include "shm.h"

void usage();

int main(int argc, char *argv[]) {
	if (argc != 1) {
		if (argv[0][0] == '\0') {;}	//un moment on nous force a compiler avec Werror
		usage();
		return -1;
	}
	
	struct entree *e;
	struct salle *s;
	e = mappy(ENTRYF_NAME);
	s = mappy(SALLE_NAME);
	
	sem_wait(&e->client);
	s->fermeture = 1;
	sem_post(&e->restaurateur);
		
	unmappy(e);
	unmappy(s);
    return 0;
}

void usage() {
	char *str = "usage: ./fermeture\n";
	write(STDERR_FILENO, str, strlen(str));
}

