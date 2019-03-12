#!/usr/bin/perl

use strict;
use warnings;
use Test::More tests => 1;

package BHStest;

use BHsolver::InlineWrap (
    C => <<"EOF",
#include <black-hole-solver/black_hole_solver.h>
#include "state.h"

int overflow(char * init_state_s, int n) {
    black_hole_solver_instance_t *solver;
    if (black_hole_solver_create(&solver))
    {
        fputs("Could not initialise solver (out-of-memory)\\n", stderr);
        exit(-1);
    }
    int error_line_num;
    const int ret = black_hole_solver_read_board(solver, init_state_s, &error_line_num,
            10,
            n,
            BHS__BLACK_HOLE__BITS_PER_COL
            );
    black_hole_solver_free(solver);
    return ret;
}

int good_ret() {
    return BLACK_HOLE_SOLVER__INVALID_INPUT;
}

EOF
    l => '-lblack_hole_solver',
);

package main;

# TEST
is(
    BHStest::overflow(
        "Foundations: -\n" . ( "AH " x 20_000 ) . "AH\n",
        1_000_000_000,
    ),
    BHStest::good_ret(),
    "Function returned BLACK_HOLE_SOLVER__INVALID_INPUT",
);
