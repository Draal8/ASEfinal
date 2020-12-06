#
# Ce Makefile contient les cibles suivantes :
#
# all		: compile les programmes
# clean		: supprime les fichiers générés automatiquement
# coverage	: compile les programmes pour mesurer la couverture de code
# test		: lance tous les tests (scripts shell test-xxx.sh)
# gcov		: génère les rapports de couverture (à lancer après avoir
#		  lancé les cibles 'coverage' et 'test').
#		  Résultats dans *.gcov
# ctags		: génère un fichier tags pour la navigation avec vim.
#		  (voir http://usevim.com/2013/01/18/tags/)
#
# De plus, les cibles supplémentaires suivantes sont fournies pour
# simplifier les tâches répétitives :
#
# couverture-et-tests	: automatise les tests avec rapport de couverture
#

COV = -coverage

CFLAGS = -g -Wall -Wextra -Werror $(COVERAGE)
LDLIBS = -g -lpthread -lrt

PROGS = restaurant police convive fermeture
CHRONO = chrono

all: $(CHRONO) $(PROGS)

#
# Il vous est suggéré pour vous simplifier la vie de créer un
# fichier shm.c contenant des fonctions communes à au moins
# deux programmes et un fichier shm.h pour vos définitions.
#

$(PROGS): shm.o

shm.o: shm.h

coverage: clean
	$(MAKE) COVERAGE=$(COV)

gcov:
	gcov *.c

test: $(CHRONO) $(PROGS)
	@for i in test-*.sh ; do echo $$i ; sh timeout.sh $$i || exit 1 ; done

couverture-et-tests: clean coverage test gcov

ctags:	
	ctags *.[ch]

clean:
	rm -f *.o $(PROGS) $(CHRONO)
	rm -f *.gc*
	rm -f *.log
	rm -f *.aux
	rm -f tags core moodle.tgz

