#!/bin/bash
export PATH="$HOME/apps/test/bhs/bin/:$PATH"
(
    i=1
    while test "$i" -le 100
    do
        printf "%d\\n" "$i"
        make_pysol_freecell_board.py -F -t "$i" black_hole |
            black-hole-solve --game black_hole - > "$i".results.txt
        let ++i
    done
) 2>&1 | tee -a black_hole_range_LOG
