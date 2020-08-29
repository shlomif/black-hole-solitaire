#!/usr/bin/env perl

use 5.014;
use strict;
use warnings;
use autodie;

use Getopt::Long qw/ GetOptions /;

sub do_system
{
    my ($args) = @_;

    my $cmd = $args->{cmd};
    print "Running [@$cmd]\n";
    if ( system(@$cmd) )
    {
        die "Running [@$cmd] failed!";
    }
}

my $IS_WIN = ( $^O eq "MSWin32" );
my $SEP    = $IS_WIN ? "\\"    : '/';
my $MAKE   = $IS_WIN ? 'gmake' : 'make';
my $SUDO   = $IS_WIN ? ''      : 'sudo';

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
my $INSTALL = !$ENV{SKIP_RINUTILS_INSTALL};

if ($INSTALL)
{
    do_system(
        {
            cmd => [ qw#git clone https://github.com/shlomif/rinutils#, ]
        }
    );
    do_system(
        {
            cmd => [
                      qq#cd rinutils && mkdir B && cd B && cmake #
                    . " -DWITH_TEST_SUITE=OFF "
                    . ( defined($cmake_gen) ? qq# -G "$cmake_gen" # : "" )
                    . (
                    defined( $ENV{CMAKE_MAKE_PROGRAM} )
                    ? " -DCMAKE_MAKE_PROGRAM=$ENV{CMAKE_MAKE_PROGRAM} "
                    : ""
                    )
                    . ( $IS_WIN ? " -DCMAKE_INSTALL_PREFIX=C:/foo " : '' )
                    . qq# .. && $SUDO $MAKE install#
            ]
        }
    );
}
do_system( { cmd => [ "cmake", "--version" ] } );

if ($IS_WIN)
{
    my $CMAKE_PREFIX_PATH = join ";",
        ( map { ; $IS_WIN ? "c:$_" : $_ } ("/foo") );
    ( $ENV{CMAKE_PREFIX_PATH} //= '' ) .= ";$CMAKE_PREFIX_PATH;";
    $ENV{RINUTILS_INCLUDE_DIR} = "C:/foo/include";
}

my $CPU_ARCH = ( delete( $ENV{GCC_CPU_ARCH} ) // 'n2' );
do_system(
    {
        cmd => [
"cd black-hole-solitaire && mkdir B && cd B && $^X ..${SEP}scripts${SEP}Tatzer -l ${CPU_ARCH}t "
                . ( defined($cmake_gen) ? qq#--gen="$cmake_gen"# : "" )
                . " && $MAKE && $^X ..${SEP}c-solver${SEP}run-tests.pl"
                . ( $INSTALL ? qq# && $SUDO $MAKE install# : '' )
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

my $pytest =
" && cd dest && py.test --cov black_hole_solver --cov-report term-missing tests${SEP}";
if ($IS_WIN)
{
    $pytest = '';
}
do_system(
    {
        cmd => [
"cd black-hole-solitaire${SEP}python-bindings${SEP}cffi${SEP} && python3 python_pypi_dist_manager.py test $pytest"
        ],
    }
);
