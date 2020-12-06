#!/bin/sh

#
# Test du choix de table et de la réutilisation
#

. ./ftest.sh

REPAS=100                               # durée d'un repas, en ms

debut_restaurant $REPAS 4 2             # table 1 : 2 couverts

# On doit utiliser la table 1 (2 couverts) plutôt que la table 0 (4 couverts)
duree_convive A 1 $((REPAS-MARGE)) $((REPAS+MARGE)) || fail "durée A"
grep -iq "table 1"   $TMP.cA  || fail "A doit être à la table 1"
code_retour_convive_ok A "Code retour A invalide"

# On doit réutiliser la table 1 puisqu'elle est maintenant libre
duree_convive B 1 $((REPAS-MARGE)) $((REPAS+MARGE)) || fail "durée B"
grep -iq "table 1"   $TMP.cB  || fail "B doit être à la table 1"
code_retour_convive_ok B "Code retour B invalide"

fermeture_restaurant

fin_restaurant 2 2

logs_aux
echo "ok"
exit 0
