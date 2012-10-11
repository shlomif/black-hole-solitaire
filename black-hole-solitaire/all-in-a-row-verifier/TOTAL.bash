#!/bin/bash

function solve()
{
    deal_idx="$1"
    shift

    deal_fn="$deal_idx.all_in_a_row.board"
    sol_fn="$deal_idx.all_in_a_row.sol"
    make_pysol_freecell_board.py -t "$deal_idx" all_in_a_row > "$deal_fn"
    if all-in-a-row-solve "$deal_fn" > "$sol_fn" ; then
        perl verify_all_in_a_row.pl "$deal_fn" "$sol_fn"
    else
        echo "Unsolved";
    fi
}

solve "$1"
