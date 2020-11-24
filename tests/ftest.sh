#
# Fonctions et variables auxiliaires utilisées pour les différents
# tests.
#
# Conseil : si vous voulez exécuter "à la main" les différentes
# commandes contenues dans les fichiers test-*.sh, faites :
#       . ./ftest.sh
# pour inclure les fonctions ci-après, puis exécutez les commandes
# de test-*.sh au clavier.
#
# Si vous avez des problèmes de durée d'exécution dûs à une machine
# lente, changez la valeur de la variable d'environnement MARGE
# dans votre shell. Par exemple :
#       MARGE=60 make test
# ou :
#       export MARGE=60
#       make test
# Ne modifiez pas ici. Ce script sera exécuté sur turing.
#

MARGE=${MARGE:-40}      # en ms : devrait suffire pour des machines rapides

TEST=$(basename $0 .sh)

TMP=/tmp/$TEST-$$
LOG=$TEST.log
SUBSH="false"           # pour avoir des "fail" silencieux dans les sous-shells

DEBUG_REST=${DEBUG_REST:-1}     # récupérer DEBUG_REST si la variable existe
export DEBUG_REST

TIMEOUT=60000           # en ms : aucun test ne devrait durer autant
TIMEOUT=10000           # en ms : aucun test ne devrait durer autant

# Rediriger stderr vers le log pour récupérer les résultats des tests
# On conserve stdout inchangé, il faudra le rediriger à la demande
exec 2> $LOG            # à commenter si vous utilisez ". ./ftest.sh"

# pour le log
echo "============================================================" >&2
echo "==> commandes exécutées" >&2

set -u                  # erreur si utilisation d'une variable non définie

###############################################################################
# Ajouter les fichiers de log auxiliaires dans le fichier de log principal

logs_aux ()
{
    set +x
    (
        for f in $(ls -d /tmp/* | grep "^$TMP" | grep -v $CHRONO)
        do
            echo "============================================================"
            echo "==> Fichier de log auxiliaire $f"
            cat $f
        done
        rm -f $TMP $TMP.*
        if islinux
        then
            echo "============================================================"
            echo "==> /dev/shm"
            ls /dev/shm
        fi
    ) >&2
}

###############################################################################
# À appeler dans chaque sous-shell créé afin d'avoir un log spécifique

subsh ()
{
    [ $# != 1 ] && echo "ERREUR SYNTAXE subsh"
    local nom="$1"
    SUBSH="true"
    exec 2> $TMP.subsh-$nom
}

###############################################################################
# Une fonction qu'il vaudrait mieux ne pas avoir à appeler...

fail ()
{
    set +x
    echo "==> Échec du test '$TEST' sur '$1'."
    echo "==> Échec du test '$TEST' sur '$1'." >&2
    if [ "$SUBSH" = false ]             # log seulement dans le sh principal
    then
        # Terminer tous les processus qu'on voit dans les variables "PID_*"
        listepid=$(set | grep '^PID_' | sed -e "s/.*='//" -e "s/'$//")
        kill -HUP $listepid 2>/dev/null # certains peuvent être absents
        echo "==> Voir détails dans le fichier $LOG"
        logs_aux
        echo "==> Exit"
    fi
    exit 1
}

###############################################################################
# Teste la présence du traditionnel message : "usage: prog arg..." dans $TMP
# Renvoie vrai si trouvé, faux si pas trouvé

tu ()
{
    # Rappel: "! cmd" => inverse le code de retour de cmd
    ! grep -q "usage: " $TMP
}

###############################################################################
# Certains tests ne sont disponibles que sur Linux

islinux ()
{
    [ x"$(uname)" = xLinux ]
}

###############################################################################
# Conversion ms -> s
# Pratique pour tout gérer en ms dans les scripts de test

mstos ()
{
    [ $# != 1 ] && echo "ERREUR SYNTAXE mstos"
    local ms="$1"
    echo "scale=5;$ms/1000" | bc
}

###############################################################################
# Sleep en millisecondes. On suppose que la commande "sleep" accepte
# des nombres flottants, ce qui n'est pas POSIX (argh...) mais qui
# est vrai sur beaucoup de systèmes.

msleep ()
{
    [ $# != 1 ] && echo "ERREUR SYNTAXE msleep"
    local ms="$1"
    sleep $(mstos $ms)
}

###############################################################################
# Teste si le processus existe toujours, sinon signale l'erreur

ps_existe ()
{
    [ $# != 2 ] && echo "ERREUR SYNTAXE ps_existe"
    local pid="$1" msg="$2"
    kill -0 $pid 2> /dev/null || fail "$msg"
}

###############################################################################
# Teste si le processus est bien terminé, sinon signale l'erreur

ps_termine ()
{
    [ $# != 2 ] && echo "ERREUR SYNTAXE ps_termine"
    local pid="$1" msg="$2"
    kill -0 $pid 2> /dev/null && fail "$msg"
}

###############################################################################
# Démarrage du restaurant et terminaison

debut_restaurant ()
{
    # La commande ":" ne fait rien => utile pour laisser une trace dans le log
    : "-> debut_restaurant"

    [ $# -lt 2 ] && echo "ERREUR SYNTAXE debut_restaurant"

    local duree_repas="$1"
    shift
    local tables="$*"

    #
    # Note :
    # - prendre une photo 1 de /dev/shm
    # - démarrer le restaurant
    # - prendre une photo 2 de /dev/shm et comparer photo 2 / photo 1
    # puis plus tard :
    # - fermer le restaurant
    # - prendre une photo 3 de /dev/shm et comparer photo 3 / photo 1
    # Ce test ne peut fonctionner que sur Linux, où /dev/shm regroupe
    # les segments de mémoire partagée
    #

    if islinux
    then
        # Conserver l'état initial des shm
        ls /dev/shm > $TMP.m1
        (echo "État initial des shm" ; cat $TMP.m1) >&2
    else
        echo "Attention : test partiel. Utilisez Linux pour le test complet"
    fi

    # Créer la ou les shm
    ./restaurant $duree_repas $tables > $TMP.r 2>&1 &
    PID_R=$!

    msleep $MARGE

    # Le restaurateur est toujours là ? Pas encore crashé ?
    ps_existe $PID_R "Le restaurateur n'est plus là"

    if islinux
    then
        # Normalement, on ne devrait pas retrouver les mêmes shm
        ls /dev/shm > $TMP.m2
        (echo "État des shm après création" ; cat $TMP.m2) >&2
        cmp -s $TMP.m1 $TMP.m2 && fail "création : 'ls /dev/shm' identique"
    fi

    # La commande ":" ne fait rien => utile pour laisser une trace dans le log
    : "<- debut_restaurant"
}

fin_restaurant ()
{
    # La commande ":" ne fait rien => utile pour laisser une trace dans le log
    : "-> fin_restaurant"

    [ $# != 2 ] && echo "ERREUR SYNTAXE fin_restaurant"

    local nc="$1" ng="$2"       # nb de convives et de groupes à vérifier

    # Normalement, le restaurateur a terminé
    ps_termine $PID_R "Le restaurateur n'a pas terminé"
    wait $PID_R || fail "Le restaurteur s'est mal terminé"

    if islinux
    then
        ls /dev/shm > $TMP.m2
        (echo "État des shm à la fin" ; cat $TMP.m2) >&2
        cmp -s $TMP.m1 $TMP.m2  || fail "fin : 'ls /dev/shm' pas identique"
    fi

    grep -q "$nc conv"   $TMP.r || fail "comptage convives par le restaurateur"
    grep -q "$ng groupe" $TMP.r || fail "comptage groupes par le restaurateur"

    # La commande ":" ne fait rien => utile pour laisser une trace dans le log
    : "<- fin_restaurant"
}

fermeture_restaurant ()
{
    # La commande ":" ne fait rien => utile pour laisser une trace dans le log
    : "-> fermeture_restaurant"

    [ $# != 0 ] && echo "ERREUR SYNTAXE fermeture_restaurant"

    # Normalement, la fermeture est instantanée
    ./fermeture > $TMP.f 2>&1 &
    PID_F=$!

    msleep $MARGE
    ps_termine $PID_F "'fermeture' aurait dû se terminer rapidement"

    # La commande ":" ne fait rien => utile pour laisser une trace dans le log
    : "<- fermeture_restaurant"
}

# Pour accumuler dans les logs l'état actuel du restaurant avant fermeture
police_puis_fermeture_restaurant ()
{
    # La commande ":" ne fait rien => utile pour laisser une trace dans le log
    : "-> police_puis_fermeture"

    [ $# != 0 ] && echo "ERREUR SYNTAXE police_puis_fermeture_restaurant"

    ./police > $TMP.p 2>&1

    fermeture_restaurant

    # La commande ":" ne fait rien => utile pour laisser une trace dans le log
    : "<- police_puis_fermeture"
}


###############################################################################
# Démarrage du restaurant et terminaison

duree_convive ()
{
    [ $# != 4 ] && echo "ERREUR SYNTAXE duree_convive"
    local convive="$1" arg2="$2" min_ms="$3" max_ms="$4"

    # Sous-shell pour pouvoir exécuter cette fonction en arrière-plan
    (
        subsh "$convive"        # dans un sous-shell => log = .subsh-A/B/...
        chrono_start
        ./convive "$convive" "$arg2" > "$TMP.c$convive" 2>&1
        echo "code retour=$?" >> "$TMP.c$convive"
        chrono_stop "$min_ms" "$max_ms" "$convive n'a pas la bonne durée"
    )
}

code_retour_convive_ok ()
{
    [ $# != 2 ] && echo "ERREUR SYNTAXE code_retour_convive_ok"
    local convive="$1" msg="$2"
    local r="$(sed -n 's/^code retour=//p' "$TMP.c$convive")"

    [ "$r" = 0 ] || fail "$msg"
}

##############################################################################
# Une sorte de chronomètre
#
# La commande date ne permet pas de récupérer l'heure avec une précision
# supérieure à la seconde (si l'on s'en tient à POSIX). On se fait donc
# la nôtre, dont les unités sont toutes en ms.
# Cette commande est ici placée dans un fichier à part, compilé avec Makefile
#

CHRONO=./chrono         # notre chronomètre (déjà compilé via Makefile)

init_chrono ()
{
    [ ! -x $CHRONO ] && fail "Il faut compiler '$CHRONO' (cf Makefile)"
}

# Démarrer le chronomètre
chrono_start ()
{
    [ $# != 0 ] && echo "ERREUR SYNTAXE chrono_start"
    init_chrono
    CHRONO_DEBUT=$($CHRONO)
}

# Arrêter le chronomètre et vérifier que la durée passée en paramètre
# est dans l'intervalle spécifié
# code retour = 0 (ok) ou 1 (erreur : durée hors de l'intervalle)
chrono_stop ()
{
    [ $# != 3 ] && echo "ERREUR SYNTAXE chrono_stop"
    local min_ms="$1" max_ms="$2" msg="$3"
    $CHRONO $CHRONO_DEBUT "$min_ms" "$max_ms" >&2 || fail "$msg"
}

chrono_duree ()
{
    $CHRONO $CHRONO_DEBUT
}

#
# Le script timeout.sh nous envoie le signal SIGALRM en cas de délai dépassé
# Attention : il peut rester des processus lancés en avant-plan et
# qui restent donc bloqués ("fail" ne peut les terminer car leur pid
# n'est pas mémorisé dans une variable PID_xxxx).
#
trap "date >&2 ; fail 'timeout dépassé'" ALRM

set -x                  # mode trace
