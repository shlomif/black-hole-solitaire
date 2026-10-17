#!/bin/bash
export PATH="$HOME/apps/perl/modules/local/bin:$PATH"
PERL5LIB+=":$HOME/apps/perl/modules/lib/perl5/site_perl/5.10.1/"
export PERL5LIB
(
i=1
while test "$i" -le 20
do
    printf "%d\\n" "$i"
    make_pysol_freecell_board.py -F -t "$i" black_hole |
        black-hole-solve - > "$i".results.txt
    let ++i
done
) 2>&1 | tee -a black_hole_range_LOG
