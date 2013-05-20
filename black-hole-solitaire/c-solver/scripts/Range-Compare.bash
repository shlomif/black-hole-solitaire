#!/bin/bash

# game="all_in_a_row"
game="black_hole"

f()
{
    local idx="$1"
    shift

    local args="$1"
    shift

    make_pysol_freecell_board.py -t -F "$idx" "$game" |
    ./black-hole-solve --game "$game" --display-boards $args |
        cat # grep -vP '^(Total number of states checked|This scan generated)'
}

for idx in `seq 1 1000` ; do
    echo "Idx: $idx"
    diff -u <(f "$idx" "") <(f "$idx" "--rank-reach-prune")
done
