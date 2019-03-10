#!/usr/bin/env bash
set -e
if ! test -d boards
then
    mkdir -p boards
    gen-multiple-pysol-layouts --dir boards/ --game black_hole --prefix bh --suffix .board seq 1 10000
fi
run()
{
    time ./multi-bhs-solver --game black_hole --quiet "$@" boards/bh{1..500}.board
}
run --rank-reach-prune
run
