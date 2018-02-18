#!/usr/bin/perl

use strict;
use warnings;

use Test::More;
use Carp;
use Data::Dumper;
use String::ShellQuote;
use File::Spec;
use File::Temp qw( tempdir );
use File::Spec::Functions qw( catpath splitpath rel2abs );

my $IS_WIN = ( $^O eq "MSWin32" );

if ($IS_WIN)
{
    plan skip_all => 'valgrind is not available on Windows';
}
else
{
    plan tests => 5;
}

my $bin_dir = catpath( ( splitpath( rel2abs $0 ) )[ 0, 1 ] );

use Games::Solitaire::Verify::Solution;

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

    system( "valgrind", "--track-origins=yes", "--leak-check=yes",
        "--log-file=$log_fn", $ENV{'FCS_PATH'} . "/$prog",
        @$cmd_line_args, );

    open my $read_from_valgrind, "<", $log_fn
        or die "Cannot open valgrind.log for reading";
    my $found_error_summary = 0;
    my $found_malloc_free   = 0;
LINES_LOOP:
    while ( my $l = <$read_from_valgrind> )
    {
        if ( index( $l, q{ERROR SUMMARY: 0 errors from 0 contexts} ) >= 0 )
        {
            $found_error_summary = 1;
        }
        elsif ( index( $l, q{in use at exit: 0 bytes} ) >= 0 )
        {
            $found_malloc_free = 1;
        }
    }
    close($read_from_valgrind);

    if ( ok( ( $found_error_summary && $found_malloc_free ), $blurb ) )
    {
        unlink($log_fn);
    }
    else
    {
        die "Valgrind failed";
    }
}

# TEST
test_using_valgrind(
    [
        '--game', 'all_in_a_row',
        File::Spec->catfile( $bin_dir, 'data', '24.all_in_a_row.board.txt' ),
    ],
    qq{valgrind all_in_a_row deal #24.}
);

# TEST
test_using_valgrind(
    [
        '--game', 'all_in_a_row', '--display-boards', '--rank-reach-prune',
        File::Spec->catfile( $bin_dir, 'data', '24.all_in_a_row.board.txt' ),
    ],
    qq{valgrind --display-boards --rank-reach-prune all_in_a_row deal #24.}
);

# TEST
test_using_valgrind(
    [
        '--game', 'black_hole',
        'non-existent-board----------flakmuttterputter.board',
    ],
    qq{valgrind does not crash on non-existent board.}
);

# TEST
test_using_valgrind(
    [
        '--game',
        'black_hole',
        '--display-boards',
        '--rank-reach-prune',
        File::Spec->catfile(
            $bin_dir, 'data', '26464608654870335080.bh.board.txt',
        ),
    ],
    qq{valgrind --display-boards --rank-reach-prune black_hole deal.}
);

# TEST
test_using_valgrind(
    {
        prog => './black-hole-solve-resume-api',
        argv => [
            '--game',
            'black_hole',
            '--iters-display-step',
            '1100',
            File::Spec->catfile(
                $bin_dir, "data", "26464608654870335080.bh.board.txt"
            )
        ],
    },
    qq{valgrind --iters-display-step resume api},
);

=head1 COPYRIGHT AND LICENSE

Copyright (c) 2009 Shlomi Fish

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

=cut

