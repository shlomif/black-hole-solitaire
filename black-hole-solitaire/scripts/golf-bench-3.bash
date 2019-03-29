#!/usr/bin/env bash
set -e
if ! test -d boards
then
    mkdir -p boards
    gen-multiple-pysol-layouts --dir boards/ --game golf --prefix golf --suffix .board seq 1 10000
fi
time ./multi-bhs-solver --game golf --rank-reach-prune boards/golf{1..200}.board
