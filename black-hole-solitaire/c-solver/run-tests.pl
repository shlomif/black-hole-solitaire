#!/usr/bin/env perl

use 5.014;
use strict;
use warnings;
use autodie;

use Getopt::Long qw/ GetOptions /;
use Env::Path ();
use Path::Tiny qw/ path /;

my $src_dir = path(__FILE__)->parent->absolute;

# Whether to use prove instead of runprove.
my $use_prove = $ENV{FCS_USE_TEST_RUN} ? 0 : 1;
my $num_jobs  = $ENV{TEST_JOBS};

sub _calc_prove
{
    return [
        'prove',
        ( $ENV{HARNESS_VERBOSE} ? ('-v')                       : () ),
        ( defined($num_jobs)    ? sprintf( "-j%d", $num_jobs ) : () )
    ];
}

my $exit_success;

sub run_tests
{
    my $tests = shift;

    my @cmd = ( ( $use_prove ? @{ _calc_prove() } : 'runprove' ), @$tests );
    if ( $ENV{RUN_TESTS_VERBOSE} )
    {
        print "Running [@cmd]\n";
    }

    # Workaround for Windows spawning-SNAFU.
    my $exit_code = system(@cmd);
    exit( $exit_success ? 0 : $exit_code ? (-1) : 0 );
}

my $tests_glob = "*.{t.exe,py,t}";
my $exclude_re_s;

my @execute;
GetOptions(
    '--exclude-re=s' => \$exclude_re_s,
    '--execute|e=s'  => \@execute,
    '--exit0!'       => \$exit_success,
    '--glob=s'       => \$tests_glob,
    '--prove!'       => \$use_prove,
    '--jobs|j=n'     => \$num_jobs,
) or die "Wrong opts - $!";

sub myglob
{
    return glob( shift . "/$tests_glob" );
}

{
    my $fcs_bin_path = Path::Tiny->cwd->absolute;
    local $ENV{FCS_PATH}     = $fcs_bin_path;
    local $ENV{FCS_BIN_PATH} = $fcs_bin_path;
    local $ENV{FCS_SRC_PATH} = $src_dir;

    local $ENV{FREECELL_SOLVER_QUIET} = 1;
    Env::Path->PATH->Prepend(
        Path::Tiny->cwd->child("board_gen"),
        $src_dir->child( "t", "scripts" ),
    );

    my $IS_WIN = ( $^O eq "MSWin32" );
    Env::Path->CPATH->Prepend( $src_dir, );

    Env::Path->LD_LIBRARY_PATH->Prepend($fcs_bin_path);
    if ($IS_WIN)
    {
        # For the shared objects.
        Env::Path->PATH->Append($fcs_bin_path);
    }

    my $foo_lib_dir = $src_dir->child( "t", "lib" );
    foreach my $add_lib ( Env::Path->PERL5LIB(), Env::Path->PYTHONPATH() )
    {
        $add_lib->Append($foo_lib_dir);
    }

    local $ENV{HARNESS_TRIM_FNS} = 'keep:1';

    local $ENV{HARNESS_PLUGINS} = join(
        ' ', qw(
            BreakOnFailure ColorSummary ColorFileVerdicts TrimDisplayedFilenames
            )
    );

    my $is_ninja = ( -e "build.ninja" );
    my $MAKE     = $IS_WIN ? 'gmake' : 'make';

    if ( !$is_ninja )
    {
        if ( system( $MAKE, "-s" ) )
        {
            die "$MAKE failed";
        }
    }

    my $BN = 0;
    my $FN = 1;

    # Put the valgrind tests last, because they take a long time.
    my @tests =
        sort { ( $a->[$BN] cmp $b->[$BN] ) || ( $a->[$FN] cmp $b->[$FN] ) }
        map  { [ path($_)->basename, $_ ] } (
        myglob('t'),
        (
              ( $fcs_bin_path ne $src_dir )
            ? ( myglob("$src_dir/t") )
            : ()
        ),
        );

    if ( defined($exclude_re_s) )
    {
        my $re = qr/$exclude_re_s/ms;
        @tests = grep { $_->[$BN] !~ $re } @tests;
    }
    @tests = grep { $_->[$BN] !~ /\A(?:lextab|yacctab)\.py\z/ } @tests;

    if ( !$ENV{FCS_TEST_BUILD} )
    {
        @tests = grep { $_->[$BN] !~ /build-process/ } @tests;
    }

    if ( $ENV{FCS_TEST_WITHOUT_VALGRIND} )
    {
        @tests = grep { $_->[$BN] !~ /valgrind/ } @tests;
    }

    $ENV{FCS_TEST_TAGS} //= '';

    print STDERR <<"EOF";
FCS_BIN_PATH = $ENV{FCS_BIN_PATH}
FCS_SRC_PATH = $ENV{FCS_SRC_PATH}
FCS_TEST_TAGS = <$ENV{FCS_TEST_TAGS}>
EOF

    if ( $ENV{FCS_TEST_SHELL} )
    {
        system("bash");
    }
    elsif (@execute)
    {
        system(@execute);
    }
    else
    {
        run_tests( [ map { $_->[$FN] } @tests ] );
    }
}

__END__

=head1 COPYRIGHT AND LICENSE

This file is part of Freecell Solver. It is subject to the license terms in
the COPYING.txt file found in the top-level directory of this distribution
and at http://fc-solve.shlomifish.org/docs/distro/COPYING.html . No part of
Freecell Solver, including this file, may be copied, modified, propagated,
or distributed except according to the terms contained in the COPYING file.

Copyright (c) 2000 Shlomi Fish

=cut
