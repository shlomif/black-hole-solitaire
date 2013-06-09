#!/bin/bash
export PATH="$HOME/apps/perl/modules/local/bin:$PATH"
export PERL5LIB="$HOME/apps/perl/modules/lib/perl5/site_perl/5.10.1/"
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
