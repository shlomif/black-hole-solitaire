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
    if ( system(@$cmd) )
    {
        die "Running [@$cmd] failed!";
    }
}

my $IS_WIN = ( $^O eq "MSWin32" );
my $SEP    = $IS_WIN ? "\\" : '/';
my $MAKE   = $IS_WIN ? 'gmake' : 'make';

my $cmake_gen;
GetOptions( 'gen=s' => \$cmake_gen, )
    or die 'Wrong options';

local $ENV{RUN_TESTS_VERBOSE} = 1;
if ( defined $cmake_gen )
{
    $ENV{CMAKE_GEN} = $cmake_gen;
}

do_system(
    {
        cmd => [ "prove", glob("root-tests/t/*.t") ],
    }
);
do_system(
    {
        cmd => [
"cd black-hole-solitaire && mkdir B && cd B && $^X ..${SEP}c-solver${SEP}Tatzer "
                . ( defined($cmake_gen) ? qq#--gen="$cmake_gen"# : "" )
                . " && $MAKE && $^X ..${SEP}c-solver${SEP}run-tests.pl"
        ]
    }
);

do_system(
    {
        cmd => [
"cd black-hole-solitaire${SEP}Games-Solitaire-BlackHole-Solver && dzil test --all"
        ]
    }
);
