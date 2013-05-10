#!/bin/bash
MAX_ITERS="40,000,000"
START=1
END="1,000,000"

    (seq "${START//,/}" "${END//,/}") |
    (
        while read T ; do
            FN="$T.rs"
            if test '!' -e "$FN" || test -z "$FN" ; then
                echo "$T"
                make_pysol_freecell_board.py -F -t "$T" black_hole |
                black-hole-solve --max-iters "${MAX_ITERS//,/}" - > "$FN"
            fi
        done
    ) 2>&1 | tee -a black_hole_fill_empty_LOG
