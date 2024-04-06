#! /usr/bin/env perl

use strict;
use warnings;
use 5.014;
use autodie;
use bytes;

use Carp         qw/ confess /;
use Getopt::Long qw/ GetOptions /;
use Path::Tiny   qw/ cwd path /;

# use lib "./lib";

use Games::Solitaire::Verify::Golf 0.2600 ();

sub run
{
    my $output_fn;

    GetOptions( "output|o=s" => \$output_fn, )
        or confess("error in cmdline args: $!");

    if ( !defined($output_fn) )
    {
        # confess("Output filename not specified! Use the -o|--output flag!");
    }

    my $BASE_DIR = $ENV{HOME};

    # my $D = "$BASE_DIR/Games-Solitaire-BlackHole-Solver";
    my $D = "$BASE_DIR/Arcs/temp/binary-star";

    my $out_fh = *STDOUT;
    $out_fh->autoflush(1);
DEALS:
    foreach my $deal ( 1 .. 10_000 )
    {
        my $board = path("$D/boards/binary_star$deal.board");
        my $sol   = path("$D/solutions/binary_star$deal.sol");
        if ( ( ( -s $sol ) // 0 ) < 500 )
        {
            next DEALS;
        }
        my $verifier = Games::Solitaire::Verify::Golf->new(
            {
                board_string => path($board)->slurp_raw(),
                variant      => "binary_star",
            }
        );

        open my $fh, '<:raw', $sol;
        $verifier->process_solution( sub { my $l = <$fh>; chomp $l; return $l; }
        );
        my $msg =
            sprintf( "Solution for deal no. %d appears to be valid.\n",
            $deal, );
        $out_fh->print($msg);
        close $fh;
    }

    exit(0);
}

run();

1;

__END__

=encoding UTF-8

=head1 ABOUT

A command-line Binary Star solitaire's solutions range verifier.

See: L<https://github.com/shlomif/black-hole-solitaire/issues/8> .

=head1 COPYRIGHT AND LICENSE

This software is Copyright (c) 2024 by Shlomi Fish.

This is free software, licensed under:

  The MIT (Expat) License

=cut
