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
my $skip_pypi = 0;
GetOptions( 'gen=s' => \$cmake_gen, 'skip-pypi!' => \$skip_pypi, )
    or die 'Wrong options';

local $ENV{RUN_TESTS_VERBOSE} = 1;
if ( defined $cmake_gen )
{
    $ENV{CMAKE_GEN} = $cmake_gen;
}

if ( !$IS_WIN )
{
    $ENV{PATH} .= ":$ENV{HOME}/.local/bin";

    # See: https://pypi.org/project/tox/
    # A Python tester - must be invocable
    do_system(
        {
            cmd => [ "which", "tox", ],
        }
    );
}

do_system(
    {
        cmd => [ "prove", glob("root-tests/t/*.t") ],
    }
);
my $INSTALL = !$ENV{SKIP_RINUTILS_INSTALL};

sub _refresh_dir
{
    my ($dir) = @_;
    return "rm -fr $dir && mkdir $dir && cd $dir";
}

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
                      qq#cd rinutils && @{[_refresh_dir('B')]} && cmake #
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

foreach my $SIGNED_CHARS_ARGS (
    [ "--cmakedefine", "USE_SIGNED_CHARS:BOOL=TRUE", ],
    [ "--cmakedefine", "USE_UNSIGNED_CHARS:BOOL=TRUE", ],
    )
{
    foreach my $config_record (
        {
            dir         => "B",
            tatzer_args => [],
        },
        {
            dir         => "B_with_max_num_played",
            tatzer_args => [
                qw/ --cmakedefine ENABLE_DISPLAYING_MAX_NUM_PLAYED_CARDS:BOOL=TRUE /
            ],
        },
        )
    {
        my ( $dir, $tatzer_args ) = @{$config_record}{qw/ dir tatzer_args /};
        $tatzer_args = [ @$tatzer_args, @$SIGNED_CHARS_ARGS, ];
        do_system(
            {
                cmd => [
"cd black-hole-solitaire && @{[_refresh_dir($dir)]} && $^X ..${SEP}scripts${SEP}Tatzer @$tatzer_args -l ${CPU_ARCH}t "
                        . ( defined($cmake_gen) ? qq#--gen="$cmake_gen"# : "" )
                        . " && $MAKE && $^X ..${SEP}c-solver${SEP}run-tests.pl"
                        . ( $INSTALL ? qq# && $SUDO $MAKE install# : '' )
                ]
            }
        );
    }
    my $dzil = sub {
        my $cmd = shift;
        return do_system(
            {
                cmd => [
"cd black-hole-solitaire${SEP}Games-Solitaire-BlackHole-Solver && $cmd"
                ]
            }
        );
    };
    $dzil->("dzil authordeps --missing | cpanm --notest");
    $dzil->("dzil listdeps --missing | cpanm --notest");
    $dzil->("dzil test --all");

    if ( not $skip_pypi )
    {
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
    }
}
