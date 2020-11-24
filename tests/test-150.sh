#!/bin/sh

#
# Test du contrôle de police avec des repas terminés et en cours
#

. ./ftest.sh

REPAS=200                               # durée d'un repas, en ms
DUREE_POLICE=$MARGE                     # en ms

debut_restaurant $REPAS 4 2             # table 1 : 2 couverts

# idem test du choix de la table
duree_convive A 1 $((REPAS-MARGE)) $((REPAS+MARGE)) || fail "durée A"
grep -iq "table 1"   $TMP.cA  || fail "A doit être à la table 1"
duree_convive B 1 $((REPAS-MARGE)) $((REPAS+MARGE)) || fail "durée B"
grep -iq "table 1"   $TMP.cB  || fail "B doit être à la table 1"

# on commence à différer ici : C est en train de manger pendant le contrôle
duree_convive C 1 $((REPAS-MARGE)) $((REPAS+MARGE)) &
PID_CC=$!

msleep $MARGE

# Triiii !
chrono_start
./police  > $TMP.p 2>&1
chrono_stop 0 $DUREE_POLICE "durée police trop longue"

# laisser à C le temps de terminer son repas
msleep $REPAS

fermeture_restaurant

fin_restaurant 3 3

# vérifications (après la fin du restaurant pour ne pas laisser les shm en vie)
code_retour_convive_ok A "Code retour A invalide"
code_retour_convive_ok B "Code retour B invalide"
wait $PID_CC || fail "C s'est mal terminé"
grep -iq "table 1"   $TMP.cC  || fail "C doit être à la table 1"
code_retour_convive_ok C "Code retour C invalide"

# Vérification du contrôle
grep -q "able 0.*vide" $TMP.p  || fail "table 0 devrait être vide"
grep -q "able 1.* C"   $TMP.p  || fail "C pas sur la table 1"
grep -q "roupe 1.* A"  $TMP.p  || fail "A pas dans le groupe 1"
grep -q "roupe 2.* B"  $TMP.p  || fail "B pas dans le groupe 2"
grep -q "roupe 3.* C"  $TMP.p  || fail "C pas dans le groupe 3"

logs_aux
echo "ok"
exit 0
