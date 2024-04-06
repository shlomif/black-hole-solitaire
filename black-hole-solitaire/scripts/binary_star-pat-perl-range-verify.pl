#! /usr/bin/env perl
#
# Short description for r.pl
#
# Version 0.0.1
# Copyright (C) 2024 Shlomi Fish < https://www.shlomifish.org/ >
#
# Licensed under the terms of the MIT license.

use strict;
use warnings;
use 5.014;
use autodie;
use bytes;

use Carp                                   qw/ confess /;
use Getopt::Long                           qw/ GetOptions /;
use Path::Tiny                             qw/ cwd path tempdir tempfile /;
use Docker::CLI::Wrapper::Container v0.0.4 ();

# use lib "./lib";

use Games::Solitaire::Verify::Golf 0.2600 ();

sub run
{
    my $output_fn;

    my $obj = Docker::CLI::Wrapper::Container->new(
        { container => "rinutils--deb--test-build", sys => "debian:sid", } );
    GetOptions( "output|o=s" => \$output_fn, )
        or confess("error in cmdline args: $!");

    if ( !defined($output_fn) )
    {
        # confess("Output filename not specified! Use the -o|--output flag!");
    }

    # $obj->do_system( { cmd => [ "git", "clone", "-b", $BRANCH, $URL, ] } );
    #
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

=head1 COPYRIGHT AND LICENSE

This software is Copyright (c) 2024 by Shlomi Fish.

This is free software, licensed under:

  The MIT (Expat) License

=cut
