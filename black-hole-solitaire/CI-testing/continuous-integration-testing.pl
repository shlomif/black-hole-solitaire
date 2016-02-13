#!/usr/bin/env perl

use strict;
use warnings;
use autodie;

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

do_system({cmd => ["cd black-hole-solitaire/ && mkdir B && cd B && ../c-solver/Tatzer && make && $^X ../c-solver/run-tests.pl"]});

do_system({cmd => ["cd black-hole-solitaire/Games-Solitaire-BlackHole-Solver/ && dzil test --all"]});
