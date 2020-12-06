#!/bin/sh

#
# Test de la durée des repas, en présence de convives arrivant les
# uns à la suite des autres.
#

. ./ftest.sh

REPAS=400                               # durée d'un repas, en ms
INTERVALLE=50

init_chrono

debut_restaurant $REPAS 5 4 2

# A attend que B et C arrivent
d=$((REPAS+2*INTERVALLE))
duree_convive A 3 $((d-MARGE)) $((d+MARGE)) &
PID_CA=$!

msleep $INTERVALLE

# B rejoint A et les deux attendent que C arrive
d=$((REPAS+1*INTERVALLE))
duree_convive B A $((d-MARGE)) $((d+MARGE)) &
PID_CB=$!

msleep $INTERVALLE

# C arrive ! Enfin !
d=$((REPAS+0*INTERVALLE))
duree_convive C A $((d-MARGE)) $((d+MARGE)) || fail "durée C"

# sur un système chargé, on pourrait fermer le restaurant avant que
# C n'ait eu le temps de s'installer à la table de A et B
msleep $MARGE

police_puis_fermeture_restaurant

fin_restaurant 3 1

wait $PID_CA || fail "A s'est mal terminé"
wait $PID_CB || fail "B s'est mal terminé"

# Vérifications
grep -iq "table 1"   $TMP.cA  || fail "A doit être à la table 1"
grep -iq "table 1"   $TMP.cB  || fail "B doit être à la table 1"
grep -iq "table 1"   $TMP.cC  || fail "C doit être à la table 1"
code_retour_convive_ok A "Code retour A invalide"
code_retour_convive_ok B "Code retour B invalide"
code_retour_convive_ok C "Code retour C invalide"

logs_aux
echo "ok"
exit 0
