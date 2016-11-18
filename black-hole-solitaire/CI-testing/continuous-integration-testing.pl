#!/usr/bin/env perl

use strict;
use warnings;
use autodie;

use Getopt::Long qw/GetOptions/;

sub do_system
{
    my ($args) = @_;

    my $cmd = $args->{cmd};
    print "Running [@$cmd]";
    if (system(@$cmd))
    {
        die "Running [@$cmd] failed!";
    }
}

my $SEP = ($^O eq "MSWin32") ? "\\" : '/';

my $cmake_gen;
GetOptions(
    'gen=s' => \$cmake_gen,
) or die 'Wrong options';
do_system({cmd => ["cd black-hole-solitaire && mkdir B && cd B && $^X ..${SEP}c-solver${SEP}Tatzer " . (defined($cmake_gen) ? qq#--gen="$cmake_gen"# : "") . " && make && $^X ..${SEP}c-solver${SEP}run-tests.pl"]});

do_system({cmd => ["cd black-hole-solitaire${SEP}Games-Solitaire-BlackHole-Solver && dzil test --all"]});
