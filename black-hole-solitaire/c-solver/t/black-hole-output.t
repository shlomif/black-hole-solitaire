#!/usr/bin/perl

use strict;
use warnings;

use Test::More tests => 20;
use Test::Differences;

use Test::Trap qw(
    trap $trap :flow:stderr(systemsafe):stdout(systemsafe):warn
);

use Socket qw(:crlf);
use Path::Tiny qw/ path /;

my $bin_dir   = path(__FILE__)->parent->absolute;
my $data_dir  = $bin_dir->child('data');
my $texts_dir = $data_dir->child('texts');

sub _normalize_lf
{
    my ($s) = @_;
    $s =~ s#$CRLF#$LF#g;
    return $s;
}

trap
{
    system( './black-hole-solve', '--game', 'black_hole',
        $data_dir->child("26464608654870335080.bh.board.txt") );
};

# TEST
ok( !( $trap->exit ), "Running the program successfully." );

use Dir::Manifest ();
my $mani = Dir::Manifest->new(
    {
        manifest_fn => $texts_dir->child('list.txt'),
        dir         => $texts_dir->child('texts'),
    }
);

# TEST
eq_or_diff(
    _normalize_lf( $trap->stdout() ),
    $mani->text( "26464608654870335080.bh.sol.txt", { lf => 1 } ),
    "Right output."
);

trap
{
    system( './black-hole-solve', '--game', 'black_hole',
        $data_dir->child("1.bh.board.txt") );
};

my $expected_output = <<'EOF';
Unsolved!


--------------------
Total number of states checked is 8.
This scan generated 8 states.
EOF

# TEST
eq_or_diff(
    _normalize_lf( $trap->stdout() ),
    _normalize_lf($expected_output),
    "Right output."
);

trap
{
    system( './black-hole-solve', '--game', 'black_hole', "--max-iters",
        "10000", $data_dir->child("26464608654870335080.bh.board.txt") );
};

# TEST
ok( !( $trap->exit ), "Running --max-iters program successfully." );

# TEST
eq_or_diff(
    _normalize_lf( $trap->stdout() ),
    $mani->text(
        "26464608654870335080-with-max-iters.bh.sol.txt", { lf => 1 }
    ),
    "Right output."
);

trap
{
    system( './black-hole-solve', '--game', 'black_hole',
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
    _normalize_lf( $trap->stdout() ),
    _normalize_lf($expected_output),
    "Right output for --max-iters."
);

my $ret_code;
trap
{
    $ret_code = system( './black-hole-solve', '--version' );
};

# TEST
is( $ret_code, 0, "Exited successfully." );

# TEST
like(
    _normalize_lf( $trap->stdout() ),
qr/\Ablack-hole-solver version (\d+(?:\.\d+){2})\r?\nLibrary version \1\r?\n\z/,
    "Right otuput for --version."
);

trap
{
    system( './black-hole-solve', '--game', 'black_hole',
        '--iters-display-step', '1000',
        $data_dir->child("26464608654870335080.bh.board.txt") );
};

# TEST
ok( !( $trap->exit ), "iters-display-step: running the program successfully." );

# TEST
eq_or_diff(
    _normalize_lf( $trap->stdout() ),
    $mani->text( "26464608654870335080-iters-step.bh.sol.txt", { lf => 1 } ),
    "Right output for iterations step."
);

# TEST:$c=2;
foreach my $exe ( './black-hole-solve', './black-hole-solve-resume-api', )
{

    trap
    {
        system( $exe, '--game', 'black_hole', '--iters-display-step', '1100',
            $data_dir->child("26464608654870335080.bh.board.txt"),
        );
    };

    # TEST*$c
    ok( !( $trap->exit ),
        "iters-display-step second step: running the program successfully." );

    # TEST*$c
    eq_or_diff(
        _normalize_lf( $trap->stdout() ),
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
        system( './black-hole-solve', '--game', 'black_hole',
            '--display-boards',
            $data_dir->child('26464608654870335080.bh.board.txt'),
        );
    };

    # TEST
    ok( !( $trap->exit ),
        "Exit code for --display-boards for board #26464608654870335080." );

    my $expected_prefix = _normalize_lf(<<'EOF');
Solved!

[START BOARD]
Foundations: AS
: KD JH JS
: 8H 4C 7D
: 7H TD 4H
: JD 9S 5S
: AH 3S 6H
: 9C 9D 8S
: 7S 2H 6S
: AC JC QH
: QD 4S TS
: 6C QS QC
: 8D 3D KH
: 5H 5C 8C
: 4D KC TC
: 6D 3C 3H
: 2C KS TH
: AD 5D 7C
: 9H 2S 2D
[END BOARD]


Move a card from stack 16 to the foundations

Info: Card moved is 2D


====================


[START BOARD]
Foundations: 2D
: KD JH JS
: 8H 4C 7D
: 7H TD 4H
: JD 9S 5S
: AH 3S 6H
: 9C 9D 8S
: 7S 2H 6S
: AC JC QH
: QD 4S TS
: 6C QS QC
: 8D 3D KH
: 5H 5C 8C
: 4D KC TC
: 6D 3C 3H
: 2C KS TH
: AD 5D 7C
: 9H 2S
[END BOARD]


Move a card from stack 13 to the foundations

Info: Card moved is 3H


====================


[START BOARD]
Foundations: 3H
: KD JH JS
: 8H 4C 7D
: 7H TD 4H
: JD 9S 5S
: AH 3S 6H
: 9C 9D 8S
: 7S 2H 6S
: AC JC QH
: QD 4S TS
: 6C QS QC
: 8D 3D KH
: 5H 5C 8C
: 4D KC TC
: 6D 3C
: 2C KS TH
: AD 5D 7C
: 9H 2S
[END BOARD]


Move a card from stack 16 to the foundations

Info: Card moved is 2S


====================


[START BOARD]
Foundations: 2S
: KD JH JS
: 8H 4C 7D
: 7H TD 4H
: JD 9S 5S
: AH 3S 6H
: 9C 9D 8S
: 7S 2H 6S
: AC JC QH
: QD 4S TS
: 6C QS QC
: 8D 3D KH
: 5H 5C 8C
: 4D KC TC
: 6D 3C
: 2C KS TH
: AD 5D 7C
: 9H
[END BOARD]


Move a card from stack 13 to the foundations

EOF

    my $stdout = _normalize_lf( $trap->stdout() );

    my $got_prefix = substr( $stdout, 0, length($expected_prefix) );

    # TEST
    eq_or_diff( _normalize_lf($got_prefix), _normalize_lf($expected_prefix),
        "Foundation in black_hole is AS rather than 0S with --display-boards."
    );

    my $expected_stdout =
        $data_dir->child( '26464608654870335080.bh-sol-with-display-boards.txt',
    )->slurp_utf8;

    # TEST
    eq_or_diff(
        _normalize_lf($stdout),
        _normalize_lf($expected_stdout),
        "Complete Right output from black_hole solver with --display-boards."
    );
}

{
    trap
    {
        system( './black-hole-solve', '--game', 'golf',
            '--display-boards', '--wrap-ranks',
            $data_dir->child('906.golf.board.txt'),
        );
    };

    # TEST
    ok( !( $trap->exit ),
        "Exit code for --display-boards for golf board #906." );

    my @cards =
        qq/6D KC KS AH AC KD 4S 8D 8H JD TC AD QH 4C JS 2C/ =~ /(\S\S)/g;
    my @strings = map { "Deal talon\n\nInfo: Card moved is $_\n" } @cards;

    my $stdout = _normalize_lf( $trap->stdout() );

    # TEST
    eq_or_diff(
        [ $stdout =~ /^(Deal talon\n\nInfo: Card moved is ..\n)/gms ],
        [@strings], "in order and correct.",
    );

    # TEST
    eq_or_diff(
        $stdout,
        _normalize_lf( $data_dir->child('906.golf.solution.txt')->slurp_utf8 ),
        "the right golf no. 906 solution",
    );
}
