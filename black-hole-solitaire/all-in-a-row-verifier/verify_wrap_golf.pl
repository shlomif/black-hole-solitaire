#!/usr/bin/perl

use strict;
use warnings;

use lib '.';
use Games::Solitaire::Verify::All_in_a_Row ();
use IO::All qw/ io /;

my ( $board_fn, $solution_fn ) = @ARGV;
my $verifier = Games::Solitaire::Verify::All_in_a_Row->new(
    {
        board_string    => scalar( io->file($board_fn)->slurp() ),
        variant         => "golf",
        wrap_ranks      => 1,
        queens_on_kings => 1,
    }
);

my $fh = io->file($solution_fn);
$verifier->process_solution( sub { return $fh->chomp->getline() } );
print "Solution is OK.\n";
exit(0);
