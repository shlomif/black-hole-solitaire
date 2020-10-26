package Games::Solitaire::BlackHole::Solver::App::Base;

use Moo;
use Getopt::Long qw/ GetOptions /;
use Pod::Usage qw/ pod2usage /;
use Math::Random::MT ();
use List::Util 1.34 qw/ any max /;

extends('Exporter');

has [
    '_active_record',
    '_active_task',
    '_board_cards',
    '_board_lines',
    '_board_values',
    '_init_foundation',
    '_init_queue',
    '_init_tasks_configs',
    '_is_good_diff',
    '_maximal_num_played_cards__from_all_tasks',
    '_prelude',
    '_prelude_iter',
    '_prelude_string',
    '_talon_cards',
    '_positions',
    '_quiet',
    '_output_handle',
    '_output_fn',
    '_should_show_maximal_num_played_cards',
    '_tasks',
    '_tasks_by_names',
    '_task_idx',
] => ( is => 'rw' );
our %EXPORT_TAGS = ( 'all' => [qw($card_re)] );
our @EXPORT_OK   = ( @{ $EXPORT_TAGS{'all'} } );

my @ranks      = ( "A", 2 .. 9, qw(T J Q K) );
my %ranks_to_n = ( map { $ranks[$_] => $_ } 0 .. $#ranks );

sub _RANK_KING
{
    return $ranks_to_n{'K'};
}

my $card_re_str = '[' . join( "", @ranks ) . '][HSCD]';
our $card_re = qr{$card_re_str};

sub _get_rank
{
    shift;
    return $ranks_to_n{ substr( shift(), 0, 1 ) };
}

sub _calc_lines
{
    my $self     = shift;
    my $filename = shift;

    my @lines;
    if ( $filename eq "-" )
    {
        @lines = <STDIN>;
    }
    else
    {
        open my $in, "<", $filename
            or die
            "Could not open $filename for inputting the board lines - $!";
        @lines = <$in>;
        close($in);
    }
    chomp @lines;
    $self->_board_lines( \@lines );
    return;
}

sub _update_max_num_played_cards
{
    my $self = shift;

    foreach my $task ( @{ $self->_tasks } )
    {
        $self->_update_max_reached_q_len($task);
    }

    return;
}

sub _trace_solution
{
    my ( $self, $final_state ) = @_;
    $self->_update_max_num_played_cards();
    my $output_handle = $self->_output_handle;
    $output_handle->print("Solved!\n");

    return if $self->_quiet;

    my $state = $final_state;
    my ( $prev_state, $col_idx );

    my @moves;
LOOP:
    while ( ( $prev_state, $col_idx ) = @{ $self->_positions->{$state} } )
    {
        last LOOP if not defined $prev_state;
        push @moves,
            (
            ( $col_idx == @{ $self->_board_cards } )
            ? "Deal talon " . $self->_talon_cards->[ vec( $prev_state, 1, 8 ) ]
            : $self->_board_cards->[$col_idx]
                [ vec( $prev_state, 4 + $col_idx, 4 ) - 1 ]
            );
    }
    continue
    {
        $state = $prev_state;
    }
    print {$output_handle} map { "$_\n" } reverse(@moves);

    return;
}

sub get_max_num_played_cards
{
    my $self = shift;

    $self->_update_max_num_played_cards();

    return $self->_maximal_num_played_cards__from_all_tasks() - 1;
}

sub _my_exit
{
    my ( $self, $verdict, ) = @_;
    my $output_handle = $self->_output_handle;

    if ( !$verdict )
    {
        $output_handle->print("Unsolved!\n");
    }
    if ( $self->_should_show_maximal_num_played_cards() )
    {
        $output_handle->printf(
            "At most %u cards could be played.\n",
            $self->get_max_num_played_cards()
        );
    }

    if ( defined( $self->_output_fn ) )
    {
        close($output_handle);
    }

    exit( !$verdict );
}

sub _parse_board
{
    my ($self) = @_;
    my $lines = $self->_board_lines;

    my $found_line = shift(@$lines);

    my $init_foundation;
    if ( my ($card) = $found_line =~ m{\AFoundations: ($card_re)\z} )
    {
        $init_foundation = $self->_get_rank($card);
    }
    else
    {
        die "Could not match first foundation line!";
    }
    $self->_init_foundation($init_foundation);

    $self->_board_cards( [ map { [ split /\s+/, $_ ] } @$lines ] );
    $self->_board_values(
        [
            map {
                [ map { $self->_get_rank($_) } @$_ ]
            } @{ $self->_board_cards }
        ]
    );
    return;
}

sub _set_up_initial_position
{
    my ( $self, $talon_ptr ) = @_;

    my $init_state = "";

    vec( $init_state, 0, 8 ) = $self->_init_foundation;
    vec( $init_state, 1, 8 ) = $talon_ptr;

    my $board_values = $self->_board_values;
    foreach my $col_idx ( keys @$board_values )
    {
        vec( $init_state, 4 + $col_idx, 4 ) =
            scalar( @{ $board_values->[$col_idx] } );
    }

    # The values of $positions is an array reference with the 0th key being the
    # previous state, and the 1th key being the column of the move.
    $self->_positions( +{ $init_state => [ undef, undef, 1, 0, ], } );

    $self->_init_queue( [$init_state] );

    return;
}

sub _shuffle
{
    my ( $self, $gen, $arr ) = @_;

    my $i = $#$arr;
    while ( $i > 0 )
    {
        my $j = int( $gen->rand( $i + 1 ) );
        if ( $i != $j )
        {
            @$arr[ $i, $j ] = @$arr[ $j, $i ];
        }
        --$i;
    }
    return;
}

my $TASK_NAME_RE  = qr/[A-Za-z0-9_]+/;
my $TASK_ALLOC_RE = qr/[0-9]+\@$TASK_NAME_RE/;

sub _process_cmd_line
{
    my ( $self, $args ) = @_;

    $self->_should_show_maximal_num_played_cards(0);
    my $quiet = '';
    my $output_fn;
    my ( $help, $man, $version );
    my @tasks;

    my $push_task = sub {
        push @tasks,
            +{
            name => undef(),
            seed => 0,
            };
        return;
    };
    $push_task->();
    GetOptions(
        "o|output=s" => \$output_fn,
        "quiet!"     => \$quiet,
        "next-task"  => sub {
            $push_task->();
            return;
        },
        "prelude=s" => sub {
            my ( undef, $val ) = @_;
            if ( $val !~ /\A$TASK_ALLOC_RE(?:,$TASK_ALLOC_RE)*\z/ )
            {
                die "Invalid prelude string '$val' !";
            }
            $self->_prelude_string($val);
            return;
        },
        "task-name=s" => sub {
            my ( undef, $val ) = @_;
            if ( $val !~ /\A$TASK_NAME_RE\z/ )
            {
                die "Invalid task name '$val' - must be alphanumeric!";
            }
            $tasks[-1]->{name} = $val;
            return;
        },
        "seed=i" => sub {
            my ( undef, $val ) = @_;
            $tasks[-1]->{seed} = $val;
            return;
        },
        "show-max-num-played-cards!" => sub {
            my ( undef, $val ) = @_;
            $self->_should_show_maximal_num_played_cards($val);
            return;
        },
        'help|h|?' => \$help,
        'man'      => \$man,
        'version'  => \$version,
        %{ $args->{extra_flags} },
    ) or pod2usage(2);
    if ( @tasks == 1 )
    {
        $tasks[-1]{name} = 'default';
    }
    if ( any { !defined $_->{name} } @tasks )
    {
        die "You did not specify the task-names for some tasks";
    }
    $self->_init_tasks_configs( \@tasks );

    pod2usage(1)                                 if $help;
    pod2usage( -exitstatus => 0, -verbose => 2 ) if $man;

    if ($version)
    {
        print
"black-hole-solve version $Games::Solitaire::BlackHole::Solver::App::Base::VERSION\n";
        exit(0);
    }

    $self->_quiet($quiet);
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
    $self->_output_fn($output_fn);
    $self->_output_handle($output_handle);
    $self->_calc_lines( shift(@ARGV) );

    return;
}

sub _set_up_tasks
{
    my ($self) = @_;
    $self->_maximal_num_played_cards__from_all_tasks(0);

    my @tasks;
    my %tasks_by_names;
    foreach my $task_rec ( @{ $self->_init_tasks_configs } )
    {
        my $iseed     = $task_rec->{seed};
        my $name      = $task_rec->{name};
        my $_task_idx = @tasks;
        my $task_obj =
            Games::Solitaire::BlackHole::Solver::App::Base::Task->new(
            {
                _name            => $name,
                _seed            => $iseed,
                _gen             => Math::Random::MT->new( $iseed || 1 ),
                _remaining_iters => 100,
                _task_idx        => $_task_idx,
            }
            );
        $task_obj->_push_to_queue( $self->_init_queue );
        push @tasks, $task_obj;
        if ( exists $tasks_by_names{$name} )
        {
            die "Duplicate task-name '$name'!";
        }
        $tasks_by_names{$name} = $task_obj;
    }
    $self->_task_idx(0);
    $self->_tasks( \@tasks );
    $self->_tasks_by_names( \%tasks_by_names );
    my @prelude;
    my $process_item = sub {
        my $s = shift;
        if ( my ( $quota, $name ) = $s =~ /\A([0-9]+)\@($TASK_NAME_RE)\z/ )
        {
            if ( not exists $self->_tasks_by_names->{$name} )
            {
                die "Unknown task name $name in prelude!";
            }
            my $task_obj = $self->_tasks_by_names->{$name};
            return Games::Solitaire::BlackHole::Solver::App::Base::PreludeItem
                ->new(
                {
                    _quota     => $quota,
                    _task      => $task_obj,
                    _task_idx  => $task_obj->_task_idx,
                    _task_name => $task_obj->_name,
                }
                );
        }
        else
        {
            die "foo";
        }
    };
    if ( my $_prelude_string = $self->_prelude_string )
    {
        push @prelude,
            (
            map { $process_item->($_) }
                split /,/, $_prelude_string
            );
    }
    $self->_prelude( \@prelude );
    $self->_prelude_iter(0);
    if ( @{ $self->_prelude } )
    {
        $self->_next_task;
    }
    return;
}

sub _update_max_reached_q_len
{
    my ( $self, $task ) = @_;

    $self->_maximal_num_played_cards__from_all_tasks(
        max(
            $self->_maximal_num_played_cards__from_all_tasks,
            $task->_max_reached_queue_len
        )
    );

    return;
}

sub _next_task
{
    my ($self) = @_;
    if ( $self->_prelude_iter < @{ $self->_prelude } )
    {
        my $alloc = $self->_prelude->[ $self->{_prelude_iter}++ ];
        my $task  = $alloc->_task;
        if ( !@{ $task->_queue } )
        {
            $self->_update_max_reached_q_len($task);
            return $self->_next_task;
        }
        $task->_remaining_iters( $alloc->_quota );
        $self->_active_task($task);
        return 1;
    }
    my $tasks = $self->_tasks;
    return if !@$tasks;
    if ( !@{ $tasks->[ $self->_task_idx ]->_queue } )
    {
        splice @$tasks, $self->_task_idx, 1;
        return $self->_next_task;
    }
    my $task = $tasks->[ $self->_task_idx ];
    $self->_task_idx( ( $self->_task_idx + 1 ) % @$tasks );
    $task->_remaining_iters(100);
    $self->_active_task($task);

    return 1;
}

sub _get_next_state
{
    my ($self) = @_;

    my $l     = @{ $self->_active_task->_queue };
    my $ret   = pop( @{ $self->_active_task->_queue } );
    my $stack = $self->_active_task->_depths_stack;
    while ( @$stack and ( $stack->[-1] == $l ) )
    {
        pop @$stack;
    }
    push @$stack, ( $l - 1 );
    return $ret;
}

sub _get_next_state_wrapper
{
    my ($self) = @_;

    my $positions = $self->_positions;

    while ( my $state = $self->_get_next_state )
    {
        my $rec = $positions->{$state};
        $self->_active_record($rec);
        return $state if $rec->[2];
    }
    return;
}

sub _process_pending_items
{
    my ( $self, $_pending, $state ) = @_;

    my $rec  = $self->_active_record;
    my $task = $self->_active_task;

    if (@$_pending)
    {
        $self->_shuffle( $task->_gen, $_pending ) if $task->_seed;
        $task->_push_to_queue( [ map { $_->[0] } @$_pending ] );
        $rec->[3] += ( scalar grep { !$_->[1] } @$_pending );
    }
    else
    {
        my $parent     = $state;
        my $parent_rec = $rec;
        my $positions  = $self->_positions;

    PARENT:
        while ( ( !$parent_rec->[3] ) or ( ! --$parent_rec->[3] ) )
        {
            $parent_rec->[2] = 0;
            $parent = $parent_rec->[0];
            last PARENT if not defined $parent;
            $parent_rec = $positions->{$parent};
        }
    }
    if ( not --$task->{_remaining_iters} )
    {
        return $self->_next_task;
    }
    return 1;
}

sub _find_moves
{
    my ( $self, $_pending, $state, $no_cards ) = @_;
    my $board_values  = $self->_board_values;
    my $fnd           = vec( $state, 0, 8 );
    my $positions     = $self->_positions;
    my $_is_good_diff = $self->_is_good_diff;
    foreach my $col_idx ( keys @$board_values )
    {
        my $pos = vec( $state, 4 + $col_idx, 4 );

        if ($pos)
        {
            $$no_cards = 0;

            my $card = $board_values->[$col_idx][ $pos - 1 ];
            if ( exists( $_is_good_diff->{ $card - $fnd } ) )
            {
                my $next_s = $state;
                vec( $next_s, 0, 8 ) = $card;
                --vec( $next_s, 4 + $col_idx, 4 );
                my $exists = exists( $positions->{$next_s} );
                my $to_add = 0;
                if ( !$exists )
                {
                    $positions->{$next_s} = [ $state, $col_idx, 1, 0 ];
                    $to_add = 1;
                }
                elsif ( $positions->{$next_s}->[2] )
                {
                    $to_add = 1;
                }
                if ($to_add)
                {
                    push( @$_pending, [ $next_s, $exists ] );
                }
            }
        }
    }

    return;
}

sub _set_up_solver
{
    my ( $self, $talon_ptr, $diffs ) = @_;

    $self->_parse_board;
    $self->_set_up_initial_position($talon_ptr);
    $self->_set_up_tasks;
    $self->_is_good_diff( +{ map { $_ => 1 } map { $_, -$_ } @$diffs, } );

    return;
}

package Games::Solitaire::BlackHole::Solver::App::Base::Task;

use Moo;

has '_queue'                 => ( is => 'ro', default => sub { return []; }, );
has '_depths_stack'          => ( is => 'ro', default => sub { return []; }, );
has '_max_reached_queue_len' => ( is => 'rw', default => 0 );
has [ '_gen', '_task_idx', '_name', '_remaining_iters', '_seed', ] =>
    ( is => 'rw' );

sub _push_to_queue
{
    my ( $self, $items ) = @_;
    die if not @$items;
    push @{ $self->_queue },        @$items;
    push @{ $self->_depths_stack }, scalar( @{ $self->_queue } );
    my $l = @{ $self->_depths_stack };
    if ( $l > $self->_max_reached_queue_len() )
    {
        $self->_max_reached_queue_len($l);
    }

    return;
}

package Games::Solitaire::BlackHole::Solver::App::Base::PreludeItem;

use Moo;

has [ '_quota', '_task', '_task_idx', '_task_name', ] => ( is => 'rw' );

1;

__END__

=head1 NAME

Games::Solitaire::BlackHole::Solver::App::Base - base class.

=head1 METHODS

=head2 new

For internal use.

=head2 get_max_num_played_cards

Returns the maximal number of played cards.

=cut
