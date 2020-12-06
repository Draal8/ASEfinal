#include "shm.h"

void usage();

int main(int argc, char *argv[]) {
	if (argc != 0) {
		if (argv[0][0] == '\0') {;}	//un moment on nous force a compiler avec Werror
		usage();
		return -1;
	}
	
    return 0;
}

void usage() {
	char *str = "usage: ./fermeture\n";
	write(STDERR_FILENO, str, strlen(str));
}

