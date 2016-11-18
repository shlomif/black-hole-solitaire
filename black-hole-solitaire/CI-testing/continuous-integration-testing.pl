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

my $cmake_gen;
GetOptions(
    'gen=s' => \$cmake_gen,
) or die 'Wrong options';
do_system({cmd => ["cd black-hole-solitaire/ && mkdir B && cd B && ../c-solver/Tatzer " . (defined($cmake_gen) ? qq#--gen="$cmake_gen"# : "") . " && make && $^X ../c-solver/run-tests.pl"]});

do_system({cmd => ["cd black-hole-solitaire/Games-Solitaire-BlackHole-Solver/ && dzil test --all"]});
