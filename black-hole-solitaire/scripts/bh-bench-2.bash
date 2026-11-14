#!/usr/bin/env bash
set -e

not_needed()
{
    if ! test -d boards
    then
        mkdir -p boards
        gen-multiple-pysol-layouts --dir boards/ --game black_hole --prefix bh --suffix .board seq 1 10000
    fi
}

run()
{
    time ./stats-multi-bhs-solver --game black_hole --quiet "$@" seq 1 500
}
run --rank-reach-prune
run
