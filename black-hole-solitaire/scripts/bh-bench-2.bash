#!/usr/bin/env bash
set -e -x

boardsfn="boards.txt"

generate_board_files()
{
    if ! test -d boards
    then
        mkdir -p boards
        gen-multiple-pysol-layouts --dir boards/ --game black_hole --prefix bh --suffix .board seq 1 10000

        gen-multiple-pysol-layouts --concat --dir "${boardsfn}" --game black_hole --prefix "" --suffix "" seq 1 500
    fi
}

generate_board_files

run()
{
    # local outdir="${TMPDIR}"
    local outdir="."
    time ./stats-multi-bhs-solver --game black_hole --output "${outdir}/${outbn}" --quiet "$@" seq 1 500
}

# slow
run_perl_solver__single_process()
{
    # local outdir="${TMPDIR}"
    local outdir="."
    time perl -M Games::Solitaire::BlackHole::Solver::App -E 'my $app = Games::Solitaire::BlackHole::Solver::App->new; $app->run(); ' -- --output "${outdir}/${outbn}" --quiet "$@" readconsec "${boardsfn}" 169 1
}

run_perl_solver()
{
    # local outdir="${TMPDIR}"
    local outdir="."
    time perl -E $'for my $idx ( 1 .. 500 ) { print "idx=$idx\\n"; system(qw& perl -M Games::Solitaire::BlackHole::Solver::App -E &, \'my $app = Games::Solitaire::BlackHole::Solver::App->new; $app->run(); \', qw& -- --quiet &,  "boards/bh$idx.board", );}'
}

outbn="output.perl.txt" run_perl_solver
outbn="output.rrp500.txt" run --rank-reach-prune
outbn="output.noprune500.txt" run
