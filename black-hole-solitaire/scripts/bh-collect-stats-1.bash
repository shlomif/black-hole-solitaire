#!/usr/bin/env bash

set -e

if false
then
    if ! test -d boards
    then
        mkdir -p boards
        gen-multiple-pysol-layouts --dir boards/ --game black_hole --prefix bh --suffix .board seq 1 10000
    fi
fi

run()
{
    time ./stats-multi-bhs-solver --game black_hole --quiet "$@" seq 1 10000
    # time ./stats-multi-bhs-solver --game black_hole --quiet "$@" boards/bh{1..10000}.board
}
# run --rank-reach-prune
run
