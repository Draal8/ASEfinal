#!/bin/sh

#
# Test d'une session minimale : ouverture et fermeture
#
# (voir ftest.sh pour les fonctions debut_restaurant,
# fermeture_restaurant et fin_restaurant)
#

. ./ftest.sh

debut_restaurant 10 2                   # cf ftest.sh

fermeture_restaurant

fin_restaurant 0 0

logs_aux
echo "ok"
exit 0
