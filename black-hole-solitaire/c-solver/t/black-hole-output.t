#!/usr/bin/perl

use strict;
use warnings;

use Test::More tests => 25;
use Test::Differences qw/ eq_or_diff /;
use Test::Some;

use Test::Trap qw(
    trap $trap :flow:stderr(systemsafe):stdout(systemsafe):warn
);

use Path::Tiny qw/ path tempdir /;

use Games::Solitaire::BlackHole::Test qw/ _test_multiple_verdict_lines /;

my $bin_dir   = path(__FILE__)->parent->absolute;
my $data_dir  = $bin_dir->child('data');
my $texts_dir = $data_dir->child('texts');

my $exit_code;

sub mysys
{
    return $exit_code = system(@_);
}

trap
{
    mysys( './black-hole-solve', '--game', 'black_hole',
        $data_dir->child("26464608654870335080.bh.board.txt") );
};

# TEST
ok( !($exit_code), "Running the program successfully." );

use Dir::Manifest        ();
use Dir::Manifest::Slurp qw/ as_lf /;
my $mani = Dir::Manifest->new(
    {
        manifest_fn => $texts_dir->child('list.txt'),
        dir         => $texts_dir->child('texts'),
    }
);

# TEST
eq_or_diff(
    as_lf( $trap->stdout() ),
    $mani->text( "26464608654870335080.bh.sol.txt", { lf => 1 } ),
    "Right output."
);

trap
{
    mysys( './black-hole-solve', '--game', 'black_hole',
        $data_dir->child("1.bh.board.txt") );
};

my $expected_output = <<'EOF';
Unsolved!


--------------------
Total number of states checked is 8.
This scan generated 8 states.
EOF

# TEST
eq_or_diff( as_lf( $trap->stdout() ), as_lf($expected_output),
    "Right output." );

trap
{
    mysys( './black-hole-solve', '--game', 'black_hole', "--max-iters",
        "10000", $data_dir->child("26464608654870335080.bh.board.txt") );
};

# TEST
ok( scalar( !$exit_code ), "Running --max-iters program successfully." );

# TEST
eq_or_diff(
    as_lf( $trap->stdout() ),
    $mani->text( "26464608654870335080.bh.sol.txt", { lf => 1 } ),
    "Right output."
);

trap
{
    mysys( './black-hole-solve', '--game', 'black_hole',
        $data_dir->child("1.bh.board.txt") );
};

$expected_output = <<'EOF';
Unsolved!


--------------------
Total number of states checked is 8.
This scan generated 8 states.
EOF

# TEST
eq_or_diff(
    as_lf( $trap->stdout() ),
    as_lf($expected_output),
    "Right output for --max-iters."
);

trap
{
    mysys( './black-hole-solve', '--version' );
};

# TEST
is( $exit_code, 0, "Exited successfully." );

# TEST
like(
    as_lf( $trap->stdout() ),
qr/\Ablack-hole-solver version (\d+(?:\.\d+){2})\r?\nLibrary version \1\r?\n\z/,
    "Right otuput for --version."
);

trap
{
    mysys( './black-hole-solve', '--game', 'black_hole',
        '--iters-display-step', '1000',
        $data_dir->child("26464608654870335080.bh.board.txt") );
};

# TEST
ok( !($exit_code), "iters-display-step: running the program successfully." );

# TEST
eq_or_diff(
    as_lf( $trap->stdout() ),
    $mani->text( "26464608654870335080-iters-step.bh.sol.txt", { lf => 1 } ),
    "Right output for iterations step."
);

# TEST:$c=1;
foreach my $exe ( './black-hole-solve', )
{

    trap
    {
        mysys( $exe, '--game', 'black_hole', '--iters-display-step', '1100',
            $data_dir->child("26464608654870335080.bh.board.txt"),
        );
    };

    # TEST*$c
    ok( !($exit_code),
        "iters-display-step second step: running the program successfully." );

    # TEST*$c
    eq_or_diff(
        as_lf( $trap->stdout() ),
        $mani->text(
            "26464608654870335080-iters-step-1100.bh.sol.txt",
            { lf => 1 }
        ),
        "Right output for iterations step on a second step."
    );

}

{
    trap
    {
        mysys( './black-hole-solve', '--game', 'black_hole', '--display-boards',
            $data_dir->child('26464608654870335080.bh.board.txt'),
        );
    };

    # TEST
    ok( !($exit_code),
        "Exit code for --display-boards for board #26464608654870335080." );

    my $expected_prefix =
        $mani->text( "26464608654870335080-disp-boards.bh.sol.txt",
        { lf => 1 } );

    my $stdout = as_lf( $trap->stdout() );

    my $got_prefix = substr( $stdout, 0, length($expected_prefix) );

    # TEST
    eq_or_diff( $got_prefix, $expected_prefix,
        "Foundation in black_hole is AS rather than 0S with --display-boards."
    );

    # TEST
    eq_or_diff(
        as_lf($stdout),
        $mani->text(
            '26464608654870335080.bh-sol-with-display-boards.txt',
            { lf => 1 }
        ),
        "Complete Right output from black_hole solver with --display-boards."
    );
}

{
    trap
    {
        mysys( './black-hole-solve', '--game', 'golf',
            '--display-boards', '--wrap-ranks',
            $data_dir->child('906.golf.board.txt'),
        );
    };

    # TEST
    ok( !($exit_code), "Exit code for --display-boards for golf board #906." );

    my @cards =
        qq/6D KC KS AH AC KD 4S 8D 8H JD TC AD QH 4C JS 2C/ =~ /(\S\S)/g;
    my @strings = map { "Deal talon\n\nInfo: Card moved is $_\n" } @cards;

    my $stdout = as_lf( $trap->stdout() );

    # TEST
    eq_or_diff(
        [ $stdout =~ /^(Deal talon\n\nInfo: Card moved is ..\n)/gms ],
        [@strings], "in order and correct.",
    );

    # TEST
    eq_or_diff(
        $stdout,
        $mani->text( '906.golf.solution.txt', { lf => 1 } ),
        "the right golf no. 906 solution",
    );
}

{
    trap
    {
        mysys( './black-hole-solve', '--game', 'golf', '--rank-reach-prune',
            $data_dir->child('2.golf.board.txt'),
        );
    };

    # TEST
    ok( !($exit_code), "Exit code for for golf board #2." );

    my $stdout = as_lf( $trap->stdout() );
    $stdout =~ s/--------------------\n\K(?:[^\n]+\n){2}\z//ms;

    # TEST
    eq_or_diff(
        $stdout,
        $mani->text( '2.golf.sol.txt', { lf => 1 } ),
        "the right golf no. 2 solution",
    );
}

my $master_tmp = tempdir();
{
    my $count = 0;
    {
        my $tmp = $master_tmp->child( "multibhs-" . ++$count );
        $tmp->mkdir();

        my $out_fn = $tmp->child("golf1to20out.txt");
        trap
        {
            mysys(
                './multi-bhs-solver',
                '--output',
                $out_fn,
                '--game',
                'golf',
                '--display-boards',
                '--wrap-ranks',
                ( map { $mani->fh("golf$_.board") } 1 .. 20 )
            );
        };

        # TEST
        ok( !($exit_code),
            "Exit code for --display-boards for golf board #906." );

        my $stdout = as_lf( path($out_fn)->slurp_raw );
        $stdout =~
s#^(\[= (?:Starting|END of) file )(\S+)#$1 . path($2)->basename#egms;

        # TEST
        is(
            $stdout,
            $mani->text( "golf-1to20.sol.txt", { lf => 1 } ),
            "recycling works",
        );
    }

    {
        my $tmp = $master_tmp->child( "multibhs-" . ++$count );
        $tmp->mkdir();
        my $out_fn = $tmp->child("golf1to20out.txt");
        trap
        {
            mysys(
                './multi-bhs-solver',
                '--output',
                $out_fn,
                '--game',
                'golf',
                '--display-boards',
                '--wrap-ranks',
                ( map { $mani->fh("golf$_.board") } 1 .. 2 ),
                $tmp->child("not--------exist.c"),
                ( map { $mani->fh("golf$_.board") } 3 .. 4 ),
            );
        };

        # TEST
        ok( $exit_code, "Exit code for --display-boards for golf board #906." );

        my $stdout = as_lf( path($out_fn)->slurp_raw );
        $stdout =~
s#^(\[= (?:Starting|END of) file )(\S+)#$1 . path($2)->basename#egms;
    }
}

my $MAX_NUM_PLAYED_CARDS_RE =
    qr/\AAt most ([0-9]+) cards could be played\.\n?\z/ms;

sub _test_max_num_played_cards
{
    my ($args) = @_;
    my ( $name, $want, $input_text ) =
        @{$args}{qw/ name expected_num input_text/};
    local $Test::Builder::Level = $Test::Builder::Level + 1;
    return subtest $name => sub {
        plan tests => 2;
        my @matches = (
            grep { /$MAX_NUM_PLAYED_CARDS_RE/ }
            map  { as_lf($_) } split( /^/ms, $input_text ),
        );

        is( scalar(@matches), 1, "One line." );

        eq_or_diff(
            [
                map {
                    /$MAX_NUM_PLAYED_CARDS_RE/
                        ? ($1)
                        : ( die "not matched!" )
                } @matches
            ],
            [$want],
            "num cards moved.",
        );
    };
}
my $no_moves_deal = $mani->fh("black_hole_27.board");

# TEST
unlike(
    $no_moves_deal->slurp_raw(),
    qr#[2K][CDHS]$#ms, "No legal moves in deal black_hole_27",
);

# TEST
subtest 'max_num_played' => sub {
    plan tests => 12;
    trap
    {
        mysys( './black-hole-solve', '--game', 'black_hole',
            '--show-max-num-played-cards', $data_dir->child("1.bh.board.txt") );
    };

    ok( $exit_code, "Non-zero exit status on unsolved." );

    $expected_output = <<'EOF';
Unsolved!


--------------------
Total number of states checked is 8.
This scan generated 8 states.
At most 3 cards could be played.
EOF

    eq_or_diff(
        as_lf( $trap->stdout() ),
        as_lf($expected_output),
        "Right output."
    );

    trap
    {
        mysys( './black-hole-solve', '--game', 'black_hole', "--max-iters",
            "10000", $data_dir->child("26464608654870335080.bh.board.txt") );
    };

    ok( scalar( !$exit_code ), "Running --max-iters program successfully." );

    eq_or_diff(
        as_lf( $trap->stdout() ),
        $mani->text( "26464608654870335080.bh.sol.txt", { lf => 1 } ),
        "Right output."
    );

    trap
    {
        mysys( './black-hole-solve', '--game', 'black_hole',
            "--show-max-num-played-cards",
            $data_dir->child("26464608654870335080.bh.board.txt") );
    };

    ok( scalar( !$exit_code ),
        "Running --show-max-num-played-cards program successfully." );

    _test_max_num_played_cards(
        {
            name         => "success moves",
            expected_num => 51,
            input_text   => scalar( $trap->stdout() ),
        },
    );

    trap
    {
        mysys( './black-hole-solve', '--game', 'all_in_a_row',
            "--show-max-num-played-cards",
            $data_dir->child('24.all_in_a_row.board.txt'),
        );
    };

    ok(
        scalar( !$exit_code ),
        "Running all_in_a_row --show-max-num-played-cards program successfully."
    );

    _test_max_num_played_cards(
        {
            name         => "all_in_a_row success moves",
            expected_num => 52,
            input_text   => scalar( $trap->stdout() ),
        },
    );

    trap
    {
        mysys( './black-hole-solve', '--game', 'black_hole',
            "--show-max-num-played-cards", $no_moves_deal, );
    };

    ok( scalar($exit_code),
"Running no-moves-possible --show-max-num-played-cards program exited with an error."
    );

    _test_max_num_played_cards(
        {
            name         => "black_hole deal #27 - 0 moves",
            expected_num => 0,
            input_text   => scalar( $trap->stdout() ),
        },
    );

    {
        my $count = 0;
        my $tmp   = $master_tmp->child( "statsmultibhs-" . ++$count );
        $tmp->mkdir();

        my @deals_indexes = ( 1 .. 20 );
        my $out_fn        = $tmp->child("golf1to20out.txt");
        trap
        {
            mysys(
                './stats-multi-bhs-solver',
                '--output',
                $out_fn,
                '--game',
                'golf',
                '--display-boards',
                '--wrap-ranks',
                ( map { $mani->fh("golf$_.board") } 1 .. 20 )
            );
        };

        if (
            not ok(
                !($exit_code),
                "Exit code for --display-boards for golf boards 1-to-20 ."
            )
            )
        {
            diag( $trap->stderr() );
        }

        _test_multiple_verdict_lines(
            {
                name =>
"test multiple verdict lines for --display-boards for golf boards 1-to-20 .",
                expected_results => [
                    "Solved!", "Solved!", "Solved!",   "Solved!",
                    "Solved!", "Solved!", "Solved!",   "Solved!",
                    "Solved!", "Solved!", "Unsolved!", "Solved!",
                    "Solved!", "Solved!", "Solved!",   "Solved!",
                    "Solved!", "Solved!", "Solved!",   "Solved!",
                ],
                expected_files_checks => sub {
                    my $i       = shift;
                    my $dealidx = $deals_indexes[$i];
                    my $fn      = path(shift);
                    my $bn      = $fn->basename();

                    return ( $bn =~ m#golf\Q$dealidx\E\.board#ms );
                },
                input_lines => [ path($out_fn)->lines_utf8() ],
            }
        );

    }
    },
    '!no_max_num_played';
