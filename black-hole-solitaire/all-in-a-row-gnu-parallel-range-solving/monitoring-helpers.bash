#!/bin/bash
unalias l
l() { ls summaries/ ; }
c() { perl collect.pl ; }
t() { tail -1 SUMMARY.txt ; }
f1() { perl -lane '/([US])/ && print$1'; }
