#!/bin/sh

#
# Test de la fermeture avec attente des tables incomplètes
#

. ./ftest.sh

REPAS=200                               # durée d'un repas, en ms
INTERVALLE=50                           # entre deux convives, en ms

debut_restaurant $REPAS 4 2

# A arrive et termine son repas
duree_convive A 1 $((REPAS-MARGE)) $((REPAS+MARGE)) || fail "durée A"

# B annonce un groupe de 4 personnes
d=$((REPAS+2*INTERVALLE))
duree_convive B 4 $((d-MARGE)) $((d+MARGE)) &
PID_CB=$!

msleep $INTERVALLE

# C rejoint B
d=$((REPAS+1*INTERVALLE))
duree_convive C B $((d-MARGE)) $((d+MARGE)) &
PID_CC=$!

# D annonce un groupe de 2 personnes
duree_convive D 2 $((d-MARGE)) $((d+MARGE)) &
PID_CD=$!

msleep $INTERVALLE

# on ferme !
police_puis_fermeture_restaurant

msleep $MARGE
ps_existe $PID_R "Le restaurant n'aurait pas dû fermer si rapidement"
ps_existe $PID_CB "B devrait toujours être à table"
ps_existe $PID_CC "C devrait toujours être à table"
ps_existe $PID_CD "D devrait toujours être à table"

msleep $((REPAS))

ps_termine $PID_CB "B devrait avoir terminé"
ps_termine $PID_CC "C devrait avoir terminé"
ps_termine $PID_CD "D devrait avoir terminé"

fin_restaurant 4 3

wait $PID_CB || fail "B s'est mal terminé"
wait $PID_CC || fail "C s'est mal terminé"
wait $PID_CD || fail "D s'est mal terminé"

# vérification des affectations
grep -iq "table 1"   $TMP.cA  || fail "A doit être à la table 1"
grep -iq "table 0"   $TMP.cB  || fail "B doit être à la table 0"
grep -iq "table 0"   $TMP.cC  || fail "C doit être à la table 0"
grep -iq "table 1"   $TMP.cD  || fail "D doit être à la table 1"

logs_aux
echo "ok"
exit 0
