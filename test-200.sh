#!/bin/sh

#
# Test de la fermeture avec attente des repas déjà commencés
#

. ./ftest.sh

REPAS=400                               # durée d'un repas, en ms
INTERVALLE=40                           # entre B et C, en ms

debut_restaurant $REPAS 4 2

# A arrive et termine son repas
duree_convive A 1 $((REPAS-MARGE)) $((REPAS+MARGE)) || fail "durée A"

# B arrive et commence son repas
duree_convive B 1 $((REPAS-MARGE)) $((REPAS+MARGE)) &
PID_CB=$!

msleep $INTERVALLE

# C arrive et commence son repas
duree_convive C 1 $((REPAS-MARGE)) $((REPAS+MARGE)) &
PID_CC=$!

msleep $MARGE

# on ferme !
police_puis_fermeture_restaurant

msleep $MARGE

ps_existe $PID_R "Le restaurant n'aurait pas dû fermer si rapidement"
ps_existe $PID_CB "B devrait encore être à table"
ps_existe $PID_CC "C devrait encore être à table (1)"

msleep $((REPAS-INTERVALLE-2*MARGE-MARGE))

ps_termine $PID_CB "B aurait dû terminer son repas"
ps_existe  $PID_CC "C devrait encore être à table (2)"

msleep $INTERVALLE

ps_termine $PID_CC "C aurait dû terminer son repas"

wait $PID_CB
wait $PID_CC

fin_restaurant 3 3

# vérification des affectations
grep -iq "table 1"   $TMP.cA  || fail "A doit être à la table 1"
grep -iq "table 1"   $TMP.cB  || fail "B doit être à la table 1"
grep -iq "table 0"   $TMP.cC  || fail "C doit être à la table 0"
code_retour_convive_ok A "Code de retour A invalide"
code_retour_convive_ok B "Code de retour B invalide"
code_retour_convive_ok C "Code de retour C invalide"

logs_aux
echo "ok"
exit 0
