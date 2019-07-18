package Games::Solitaire::BlackHole::Solver::App;

use strict;
use warnings;

use 5.014;

use Getopt::Long;
use Pod::Usage;

use parent 'Games::Solitaire::BlackHole::Solver::App::Base';
use Games::Solitaire::BlackHole::Solver::App::Base qw/ $card_re /;

=head1 NAME

Games::Solitaire::BlackHole::Solver::App - a command line application
implemented as a class to solve the Black Hole solitaire.

=head1 SYNOPSIS

    use Games::Solitaire::BlackHole::Solver::App;

    my $app = Games::Solitaire::BlackHole::Solver::App->new;

    $app->run();

And then from the command-line:

    $ black-hole-solve myboard.txt

=head1 DESCRIPTION

A script that encapsulates this application accepts a filename pointing
at the file containing the board or C<"-"> for specifying the standard input.

A board looks like this and can be generated for PySol using the
make_pysol_board.py in the contrib/ .

    Foundations: AS
    KD JH JS
    8H 4C 7D
    7H TD 4H
    JD 9S 5S
    AH 3S 6H
    9C 9D 8S
    7S 2H 6S
    AC JC QH
    QD 4S TS
    6C QS QC
    8D 3D KH
    5H 5C 8C
    4D KC TC
    6D 3C 3H
    2C KS TH
    AD 5D 7C
    9H 2S 2D

Other flags:

=over 4

=item * --version

=item * --help

=item * --man

=item * -o/--output solution_file.txt

Output to a solution file.

=back

More information about Black Hole Solitaire can be found at:

=over 4

=item * L<http://en.wikipedia.org/wiki/Black_Hole_%28solitaire%29>

=item * L<http://pysolfc.sourceforge.net/doc/rules/blackhole.html>

=back

=head1 METHODS

=head2 $self->new()

Instantiates an object.

=head2 $self->run()

Runs the application.

=cut

sub run
{
    my $self      = shift;
    my $RANK_KING = $self->_RANK_KING;
    my $output_fn;

    my ( $help, $man, $version );

    GetOptions(
        "o|output=s" => \$output_fn,
        'help|h|?'   => \$help,
        'man'        => \$man,
        'version'    => \$version,
    ) or pod2usage(2);

    pod2usage(1) if $help;
    pod2usage( -exitstatus => 0, -verbose => 2 ) if $man;

    if ($version)
    {
        print
"black-hole-solve version $Games::Solitaire::BlackHole::Solver::App::VERSION\n";
        exit(0);
    }

    my $filename = shift(@ARGV);

    my $output_handle;

    if ( defined($output_fn) )
    {
        open( $output_handle, ">", $output_fn )
            or die "Could not open '$output_fn' for writing";
    }
    else
    {
        open( $output_handle, ">&STDOUT" );
    }

    my @lines = @{ $self->_calc_lines($filename) };

    my $found_line = shift(@lines);

    my $init_foundation;
    if ( my ($card) = $found_line =~ m{\AFoundations: ($card_re)\z} )
    {
        $init_foundation = $self->_get_rank($card);
    }
    else
    {
        die "Could not match first foundation line!";
    }

    $self->_board_cards( [ map { [ split /\s+/, $_ ] } @lines ] );
    my @board_values = map {
        [ map { $self->_get_rank($_) } @$_ ]
    } @{ $self->_board_cards };

    my $init_state = "";

    vec( $init_state, 0, 8 ) = $init_foundation;

    foreach my $col_idx ( 0 .. $#board_values )
    {
        vec( $init_state, 4 + $col_idx, 4 ) =
            scalar( @{ $board_values[$col_idx] } );
    }

    # The values of $positions is an array reference with the 0th key being the
    # previous state, and the 1th key being the column of the move.
    my $positions = $self->_positions( +{ $init_state => [], } );

    my @queue = ($init_state);

    my %is_good_diff = ( map { $_ => 1 } map { $_, -$_ } ( 1, $RANK_KING ) );

    my $verdict = 0;

QUEUE_LOOP:
    while ( my $state = pop(@queue) )
    {
        # The foundation
        my $fnd      = vec( $state, 0, 8 );
        my $no_cards = 1;

        # my @debug_pos;
        foreach my $col_idx ( 0 .. $#board_values )
        {
            my $pos = vec( $state, 4 + $col_idx, 4 );

            # push @debug_pos, $pos;
            if ($pos)
            {
                $no_cards = 0;

                my $card = $board_values[$col_idx][ $pos - 1 ];
                if ( exists( $is_good_diff{ $card - $fnd } ) )
                {
                    my $next_s = $state;
                    vec( $next_s, 0, 8 ) = $card;
                    --vec( $next_s, 4 + $col_idx, 4 );
                    if ( !exists( $positions->{$next_s} ) )
                    {
                        $positions->{$next_s} = [ $state, $col_idx ];
                        push( @queue, $next_s );
                    }
                }
            }
        }

        # print "Checking ", join(",", @debug_pos), "\n";
        if ($no_cards)
        {
            print {$output_handle} "Solved!\n";
            $self->_trace_solution( $state, $output_handle );
            $verdict = 1;
            last QUEUE_LOOP;
        }
    }

    return $self->_my_exit( $verdict, $output_handle, $output_fn );
}

=head1 SEE ALSO

The Black Hole Solitaire Solvers homepage is at
L<http://www.shlomifish.org/open-source/projects/black-hole-solitaire-solver/>
and one can find there an implementation of this solver as a C library (under
the same licence), which is considerably faster and consumes less memory,
and has had some other improvements.

=head1 AUTHOR

Shlomi Fish, L<http://www.shlomifish.org/>

=head1 BUGS

Please report any bugs or feature requests to
C<games-solitaire-blackhole-solver rt.cpan.org>, or through
the web interface at L<http://rt.cpan.org/NoAuth/ReportBug.html?Queue=Games-Solitaire-BlackHole-Solver>.  I will be notified, and then you'll
automatically be notified of progress on your bug as I make changes.

=head1 COPYRIGHT AND LICENSE

Copyright (c) 2010 Shlomi Fish

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

1;
