#include "shm.h"

int main() {
	int flag = 0;
	struct salle *s;
	struct registre *r;
	s = mappy(SALLE_NAME);
	r = mappy(REGISTRY);
	
	salle_dump(s, stdout);
	//faire une recherche des element de s dans r
	
	
	
	if (flag != 0) {
		printf("Vous etes en infraction monsieur\n");
		return -1;
	}
	
    return 0;
}








