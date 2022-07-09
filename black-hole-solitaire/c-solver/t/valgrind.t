#!/usr/bin/perl

use strict;
use warnings;

use Test::More;
use Test::RunValgrind ();
use Path::Tiny        qw/ path /;

my $bin_dir  = path($0)->parent->absolute;
my $data_dir = $bin_dir->child('data');

my $IS_WIN = ( $^O eq "MSWin32" );

if ($IS_WIN)
{
    plan skip_all => 'valgrind is not available on Windows';
}
else
{
    plan tests => 5;
}

sub test_using_valgrind
{
    my $args  = shift;
    my $blurb = shift;

    my $log_fn = "valgrind.log";

    if ( ref($args) eq "ARRAY" )
    {
        $args = { argv => $args, prog => "black-hole-solve", };
    }

    my $cmd_line_args = $args->{argv};
    my $prog          = $args->{prog};
    my $success       = Test::RunValgrind->new(
        {
            supress_stderr => $args->{supress_stderr},
        }
    )->run(
        {
            blurb  => $blurb,
            log_fn => $log_fn,
            prog   => $ENV{'FCS_BIN_PATH'} . "/$prog",
            argv   => [ @$cmd_line_args, ],
        },
    );
    if (0)
    {
        if ( not $success )
        {
            diag( path($log_fn)->slurp_utf8 );
            require Carp;
            Carp::confess("valgrind error");
        }
    }
    return $success;
}

# TEST
test_using_valgrind(
    [
        '--game', 'all_in_a_row', $data_dir->child('24.all_in_a_row.board.txt'),
    ],
    qq{valgrind all_in_a_row deal #24.}
);

# TEST
test_using_valgrind(
    [
        '--game',           'all_in_a_row',
        '--display-boards', '--rank-reach-prune',
        $data_dir->child('24.all_in_a_row.board.txt'),
    ],
    qq{valgrind --display-boards --rank-reach-prune all_in_a_row deal #24.}
);

# TEST
test_using_valgrind(
    {
        argv => [
            '--game', 'black_hole',
            'non-existent-board----------flakmuttterputter.board',
        ],
        prog           => 'black-hole-solve',
        supress_stderr => 1,
    },
    qq{valgrind does not crash on non-existent board.}
);

# TEST
test_using_valgrind(
    [
        '--game', 'black_hole', '--display-boards', '--rank-reach-prune',
        $data_dir->child( '26464608654870335080.bh.board.txt', ),
    ],
    qq{valgrind --display-boards --rank-reach-prune black_hole deal.}
);

# TEST
test_using_valgrind(
    {
        prog => './black-hole-solve',
        argv => [
            '--game', 'black_hole', '--iters-display-step', '1100',
            $data_dir->child("26464608654870335080.bh.board.txt")
        ],
    },
    qq{valgrind --iters-display-step resume api},
);
