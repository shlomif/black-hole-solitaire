#!/usr/bin/env bash
set -e -x

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
    # local outdir="${TMPDIR}"
    local outdir="."
    time ./stats-multi-bhs-solver --game black_hole --output "${outdir}/${outbn}" --quiet "$@" seq 1 500
}
outbn="output.rrp500.txt" run --rank-reach-prune
outbn="output.noprune500.txt" run
