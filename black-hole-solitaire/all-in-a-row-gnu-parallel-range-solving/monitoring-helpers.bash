#!/bin/bash
unalias l
l() { ls summaries/ ; }
c() { perl collect.pl ; }
t() { tail -1 SUMMARY.txt ; }
u() { myrsync SUMMARY.txt lap:Arcs/ ; myrsync SUMMARY.txt hostgator: ; }
f1() { perl -lane '/([US])/ && print $1'; }
f2() { perl -lane '$F[1] eq $ENV{V} and print $F[2]'; }
report() { local w="$1" ; shift; (export V="$w" ; paste <(f2 < SUMMARY-no-prune.txt ) <(f2 < SUMMARY-prune.txt ) > iters-"$V".txt ) ; }
