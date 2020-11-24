#!/bin/sh

#
# Test d'une session très basique : ouverture, 1 convive et fermeture
#

. ./ftest.sh

REPAS=100                               # durée d'un repas, en ms

debut_restaurant $REPAS 2 2

# Ce test diffère du précédent par le fait qu'il y a un convive
duree_convive A 1 $((REPAS-MARGE)) $((REPAS+MARGE)) || fail "durée A"

fermeture_restaurant

fin_restaurant 1 1

# vérifications (après la fin du restaurant pour ne pas laisser les shm en vie)
grep -iq "table 0"   $TMP.cA  || fail "A doit être à la table 0"
code_retour_convive_ok A "Code retour de convive A devrait être 0"

logs_aux
echo "ok"
exit 0
