#!/usr/bin/env bash
mkdir ../foo
gen-multiple-pysol-layouts --dir ../foo --ms --prefix '' --suffix .board --game black_hole seq 1 32000
(for i in ../foo/{1..32000}.board ; do perl -Ilib bin/black-hole-solve --seed 0 --seed 1 --seed 2 --seed 3 --quiet "$i" ; done) | cat -n | timestamper | tee -a new.log.txt
(for i in ../foo/{1..32000}.board ; do perl -Ilib bin/black-hole-solve --quiet "$i" ; done) | cat -n | timestamper | tee -a old.log.txt
f() { export TITLE="$2"; cat "$1" | head -34 | perl -lanE 'say +($F[0]-($x//=$F[0]))||$ENV{TITLE}'}
paste -d ,, <(seq 1 34) <(f old.log.txt Old) <(f new.log.txt New) | csv2chart xlsx -o foo.xlsx --exec gnumeric
