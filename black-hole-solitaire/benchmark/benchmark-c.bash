#!/bin/bash
export PATH="/home/shlomif/apps/test/bhs/bin/:$PATH"
    (
        seq 1 20
    ) |
    (
        while read T ; do
            echo "$T"
            make_pysol_freecell_board.py -F -t "$T" black_hole |
            black-hole-solve - > "$T".results.txt
        done
    ) 2>&1 | tee -a black_hole_range_LOG
