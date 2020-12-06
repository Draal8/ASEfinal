#!/bin/sh

#
# Test du contrôle de police simple
#

. ./ftest.sh

REPAS=100                               # durée d'un repas, en ms
DUREE_POLICE=$MARGE                     # en ms

debut_restaurant $REPAS 4 2             # table 1 : 2 couverts

# Triiii !
chrono_start
./police  > $TMP.p 2>&1
chrono_stop 0 $DUREE_POLICE "durée police trop longue"

fermeture_restaurant

fin_restaurant 0 0

# Vérification du contrôle
grep -q "able 0.*vide" $TMP.p  || fail "table 0 devrait être vide"
grep -q "able 1.*vide" $TMP.p  || fail "table 1 devrait être vide"

logs_aux
echo "ok"
exit 0
