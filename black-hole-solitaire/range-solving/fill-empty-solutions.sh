#!/bin/bash
MAX_ITERS="40,000,000"
START=1
END="1,000,000"

(
    i="${START//,/}"
    e="${END//,/}"
    max_iters="${MAX_ITERS//,/}"
    while test "$i" -le "$e"
    do
        filename="$i.rs"
        if test '!' -e "$filename" || test -z "$filename"
        then
            echo "$i"
            make_pysol_freecell_board.py -F -t "$i" black_hole |
            black-hole-solve --max-iters "${max_iters}" - > "$filename"
        fi
        let ++i
    done
) 2>&1 | tee -a black_hole_fill_empty_LOG
