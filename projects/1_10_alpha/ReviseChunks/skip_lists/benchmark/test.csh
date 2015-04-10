#!/bin/csh

foreach f (1000 10000 100000 1000000 10000000 100000000)

    echo
    echo "Testing with $f records"

    ./btree -n $f
    ./btree -n $f
    ./btree -n $f
    ./btree -n $f
    ./btree -n $f

    ls -l btree.h5

    ./btree -m read -a 800 -n $f
    ./btree -m read -a 800 -n $f
    ./btree -m read -a 800 -n $f
    ./btree -m read -a 800 -n $f
    ./btree -m read -a 800 -n $f

    rm btree.h5

    ./btree2 -n $f
    ./btree2 -n $f
    ./btree2 -n $f
    ./btree2 -n $f
    ./btree2 -n $f

    ls -l btree2.h5

    ./btree2 -m read -a 800 -n $f
    ./btree2 -m read -a 800 -n $f
    ./btree2 -m read -a 800 -n $f
    ./btree2 -m read -a 800 -n $f
    ./btree2 -m read -a 800 -n $f

    rm btree2.h5

    ./earray -n $f
    ./earray -n $f
    ./earray -n $f
    ./earray -n $f
    ./earray -n $f

    ls -l earray.h5

    ./earray -m read -a 800 -n $f
    ./earray -m read -a 800 -n $f
    ./earray -m read -a 800 -n $f
    ./earray -m read -a 800 -n $f
    ./earray -m read -a 800 -n $f

    rm earray.h5

end

