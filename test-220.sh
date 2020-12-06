#!/bin/sh

#
# Tests de stress
#

. ./ftest.sh

REPAS=200                               # durée d'un repas, en ms
INTERVALLE=50

NTABLES=30                              # nombre de packs de 2 tables

MULTMARGE=$((NTABLES / 2))              # ok sur machine récente

# La commande "seq" n'est pas POSIX : la recréer car on va en avoir besoin
seq ()
(
    set +x
    local max="$1"
    L=""
    i=1
    while [ $i -le $max ]
    do
        L="$L $i"
        i=$((i+1))
    done
    echo $L
)

#
# Préparer les tables
#

L=""
for i in $(seq $NTABLES)
do
    L="$L 6 2"
done
debut_restaurant $REPAS $L

msleep $MARGE
ps_existe $PID_R "restaurant n'est plus là"

#
# Lancer des tables incomplètes pour les tables de 2
#

MIN=$((REPAS+INTERVALLE-MARGE))
MAX=$((REPAS+INTERVALLE+MULTMARGE*MARGE))       # augmenter la marge
LPID2=""
for i in $(seq $NTABLES)
do
    duree_convive A$i 2 $MIN $MAX &
    LPID2="$LPID2 $!"
done

msleep $INTERVALLE
./police > $TMP.p1

#
# Puis lancer des tables incomplètes pour les tables de 6
#

LPID6=""
for i in $(seq $NTABLES)
do
    duree_convive C$i 2 $MIN $MAX &
    LPID6="$LPID6 $!"
done

msleep $MARGE

#
# Finir les repas des tables de 2
#
MIN=$((REPAS-MARGE))
MAX=$((REPAS+MULTMARGE*MARGE))
for i in $(seq $NTABLES)
do
    duree_convive B$i A$i $MIN $MAX &
    LPID2="$LPID2 $!"
done

#
# Organiser quelques refoulements
#
MAX=$((MULTMARGE*MARGE))
LREF=""
for i in $(seq $NTABLES)
do
    duree_convive D$i 1 0 $MAX &
    LREF="$LREF $!"
done

for i in $(seq $NTABLES)
do
    duree_convive E$i M.X 0 $MAX &
    LREF="$LREF $!"
done

wait $LREF

police_puis_fermeture_restaurant

# attendre la fin de tous les repas
msleep $((REPAS+MARGE))

fin_restaurant $((3*NTABLES)) $((2*NTABLES))

for pid in $LPID2 $LPID6
do
    wait $pid || fail "pid $pid s'est mal terminé"
done

logs_aux
echo "ok"
exit 0
