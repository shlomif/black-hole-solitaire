#!/usr/bin/perl

use strict;
use warnings;
use autodie;

use List::Util qw(first);

my $board_idx = shift(@ARGV);

my $text= `make_pysol_freecell_board.py -F -t $board_idx all_in_a_row | all-in-a-row-solve`;

my ($verdict, $checked, $gen) = ($text =~ m{(Solved|Unsolved).*?^Total number of states checked is (\d+)\..*?^This scan generated (\d+) states\.}ms);

my $v = substr($verdict, 0, 1);

# We are using a temporary place and moving so it will be atomic.
my $basename = "$board_idx.summary";
my $temp_fn = "temp-summaries/$basename";
open my $out_fh, '>', $temp_fn;
print {$out_fh} "$board_idx\t$v\t$checked\t$gen\n";
close ($out_fh);

rename ($temp_fn, "summaries/$basename");
