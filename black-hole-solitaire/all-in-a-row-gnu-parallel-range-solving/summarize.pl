#!/usr/bin/perl

use strict;
use warnings;
use autodie;

use List::Util qw(first);
use File::Copy qw(copy);
use Env::Path;

my $p = Env::Path->PATH;

my $bhs_dir = "$ENV{HOME}/apps/black-hole-solver";
$p->Prepend("$bhs_dir/bin");

my $board_idx = shift(@ARGV);

my $basename = "$board_idx.summary";
my $final_fn = "$bhs_dir/summaries/$basename";

if (-e $final_fn)
{
    exit(0);
}

my $game = 'all_in_a_row';
my @args = (qw(--rank-reach-prune));
my $text= `make_pysol_freecell_board.py -F -t $board_idx $game | black-hole-solve --game $game @args -`;

my ($verdict, $checked, $gen) = ($text =~ m{(Solved|Unsolved).*?^Total number of states checked is (\d+)\..*?^This scan generated (\d+) states\.}ms);

my $v = substr($verdict, 0, 1);

# We are using a temporary place and moving so it will be atomic.
my $temp_fn = "$bhs_dir/temp-summaries/$basename";
open my $out_fh, '>', $temp_fn;
print {$out_fh} "$board_idx\t$v\t$checked\t$gen\n";
close ($out_fh);

copy ($temp_fn, $final_fn);
unlink ($temp_fn);
