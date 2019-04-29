package Games::Solitaire::Verify::App::Golf;

use strict;
use warnings;
use autodie;

use Games::Solitaire::Verify::All_in_a_Row ();
use Path::Tiny qw/ path tempdir tempfile cwd /;

sub run
{
    my ( $self,     $args )        = @_;
    my ( $board_fn, $solution_fn ) = @ARGV;
    my $verifier = Games::Solitaire::Verify::All_in_a_Row->new(
        {
            board_string => path($board_fn)->slurp_raw(),
            %$args,
        }
    );

    open my $fh, '<', $solution_fn;
    $verifier->process_solution( sub { my $l = <$fh>; chomp $l; return $l; } );
    print "Solution is OK.\n";
    exit(0);
}

1;
