#!/bin/sh

#
# Test de refoulement sur table complète
#

. ./ftest.sh

REPAS=200                               # durée d'un repas, en ms
INTERVALLE=30

debut_restaurant $REPAS 4 3             # on n'utilise que la table 1

# groupe de 2
duree_convive A 2 $((REPAS+INTERVALLE-MARGE)) $((REPAS+INTERVALLE+MARGE)) &
PID_CA=$!

msleep $INTERVALLE

duree_convive B A $((REPAS-MARGE)) $((REPAS+MARGE)) &
PID_CB=$!

msleep $MARGE

# désolé, C, tu n'étais pas prévu au programme
duree_convive C A 0 $MARGE &
PID_CC=$!

msleep $MARGE

police_puis_fermeture_restaurant

# attendre la fin du repas de A et B
msleep $REPAS

fin_restaurant 2 1

# est-ce que tout le monde s'est bien terminé
wait $PID_CA || fail "A s'est mal terminé"
wait $PID_CB || fail "B s'est mal terminé"
wait $PID_CC || fail "C s'est mal terminé"

# normalement, le refoulement ne doit pas être vu comme une erreur
code_retour_convive_ok A "Code retour A invalide"
code_retour_convive_ok B "Code retour B invalide"
code_retour_convive_ok C "Refoulement de C != erreur"

logs_aux
echo "ok"
exit 0
