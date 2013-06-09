#!/bin/bash

board_idx=1

while true ; do
    echo "Reached board No. $board_idx"
    /home/shlomif/progs/freecell/trunk/fc-solve/source/board_gen/make_pysol_freecell_board.py -t "$board_idx" all_in_a_row > board.txt
    ./black-hole-solve board.txt > solution.txt
    if head -1 solution.txt | grep -F 'Solved' > /dev/null ; then
        if ! perl -I "../../all-in-a-row-verifier/" "../../all-in-a-row-verifier/verify_all_in_a_row.pl" board.txt solution.txt ; then
            echo "Solution for board No. $board_idx is wrong. Terminating."
            exit 1
        fi
    fi
    let board_idx++
done
