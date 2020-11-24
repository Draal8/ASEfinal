#!/bin/sh

#
# Tests basiques de vérification des arguments
#

. ./ftest.sh

###############################################################################
# Tests d'arguments invalides
# (on attend un message d'erreur du type "usage: ..." pour être sûr
# que le problème de syntaxe est bien détecté)
#
# Note : tu = "test usage", défini dans ftest.sh

./restaurant            2> $TMP >&2 || tu && fail "r: nb arg (0) invalide"
./restaurant 10         2> $TMP >&2 || tu && fail "r: nb arg (1) invalide"
./restaurant x 10       2> $TMP >&2 || tu && fail "r: arg1 chaine"
./restaurant 10 x       2> $TMP >&2 || tu && fail "r: arg2 chaine"
./restaurant 10x 10     2> $TMP >&2 || tu && fail "r: arg1 nombre invalide"
./restaurant 10 10x     2> $TMP >&2 || tu && fail "r: arg2 nombre invalide"
./restaurant 0 10       2> $TMP >&2 || tu && fail "r: arg1 == 0"
./restaurant -1 10      2> $TMP >&2 || tu && fail "r: arg1 < 0"
./restaurant 10 0       2> $TMP >&2 || tu && fail "r: arg2 == 0"
./restaurant 10 -1      2> $TMP >&2 || tu && fail "r: arg2 < 0"
./restaurant 10 7       2> $TMP >&2 || tu && fail "r: arg2 > MAX_CAPACITE"

./convive A             2> $TMP >&2 || tu && fail "c: nb arg (1) invalide"
./convive A B C         2> $TMP >&2 || tu && fail "c: nb arg (3) invalide"
./convive A2345678901 B 2> $TMP >&2 || tu && fail "c: arg1 trop long"
./convive A B2345678901 2> $TMP >&2 || tu && fail "c: arg2 trop long"
./convive "" B          2> $TMP >&2 || tu && fail "c: arg1 vide"
./convive A ""          2> $TMP >&2 || tu && fail "c: arg2 vide"
./convive A 10x         2> $TMP >&2 || tu && fail "c: arg2 nombre invalide"

./fermeture 1           2> $TMP >&2 || tu && fail "f: nb arg (1) invalide"

./police 1              2> $TMP >&2 || tu && fail "p: nb arg (1) invalide"

logs_aux
echo "ok"
exit 0
