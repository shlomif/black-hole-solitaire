#!/usr/bin/perl

use strict;
use warnings;

use Data::Dump;
use DBI;
use IO::All;
use List::MoreUtils qw(zip part);

my $DATABASE_FILE = "black_hole_solver.sqlite";

my @ranks = ("A", 2 .. 9, qw(T J Q K));
my %ranks_to_n = (map { $ranks[$_] => $_ } 0 .. $#ranks);

my @suits = (qw(H C D S));
my %suits_to_n = (map { $suits[$_] => $_ } 0 .. $#suits);

my $card_re_str = '[' . join("", @ranks) . '][HSCD]';
my $card_re = qr{$card_re_str};

my %card_str_to_chr;

foreach my $s (@suits)
{
    foreach my $r (@ranks)
    {
        # We start from 1 because '\0' may not be handled very well by
        # certain languages.
        $card_str_to_chr{$r.$s} =
            chr(1 + $suits_to_n{$s} * 13 + $ranks_to_n{$r});
    }
}

# dd(\%card_str_to_chr);

my %stack_to_chr = (map { $_ => chr(1+$_) } (0 .. 16));

my $should_init_db = (! -e $DATABASE_FILE);

my $dbh = DBI->connect("dbi:SQLite:dbname=$DATABASE_FILE", "", "");

my $runs_table = "bhs_runs";
my $solutions_table = "bhs_solutions";
if ($should_init_db)
{
    $dbh->do("CREATE TABLE $runs_table (idx INTEGER PRIMARY KEY, status CHAR(1), num_checked INTEGER, num_generated INTEGER)");
    $dbh->do("CREATE TABLE $solutions_table (idx INTEGER PRIMARY KEY, solution VARCHAR(150))");
}

sub _slurp
{
    my $filename = shift;

    open my $in, "<", $filename
        or die "Cannot open '$filename' for slurping - $!";

    local $/;
    my $contents = <$in>;

    close($in);

    return $contents;
}

my $unsolved_sth =
    $dbh->prepare("INSERT INTO $runs_table (idx, status, num_checked, num_generated) VALUES (?, 'U', ?, ?)");

my $solved_sth = 
    $dbh->prepare("INSERT INTO $runs_table (idx, status, num_checked, num_generated) VALUES (?, 'S', ?, ?)");

my $solution_sth = 
    $dbh->prepare("INSERT INTO $solutions_table (idx, solution) VALUES (?, ?)");

foreach my $deal (1 .. 1_000_000)
{
    print STDERR "Reached $deal\n";

    my $fn = "range-check/$deal.rs";

    my $text = _slurp($fn);
   
    if ($text =~ m{\AUnsolved!})
    {
        if ($text !~ m{^Total number of states checked is (\d+)\.\nThis scan generated \1 states\.$}ms)
        {
            die "Mismatching numbers in $fn.";            
        }
        else
        {
            my $num = $1;
       
            $unsolved_sth->execute($deal, $num, $num);
        }
    }
    elsif ($text =~ m{\ASolved!})
    {
        if ($text !~ m{^Total number of states checked is (\d+)\.\nThis scan generated (\d+) states\.$}ms)
        {
            die "Mismatching lines in $fn.";
        }
        else
        {
            my ($checked, $gen) = ($1, $2);
            
            $solved_sth->execute($deal, $checked, $gen);
            
            my @moves = ($text =~ m{^Move a card from stack (\d+) to the foundations\n\nInfo: Card moved is ($card_re)\n\n\n====================\n}gms);
            
            if (@moves != 51*2)
            {
                die "Incorrect number of moves in file $fn.";
            }
            my $i = 0;
            my ($stacks, $cards) = part { $i++ % 2 } @moves;
            $solution_sth->execute($deal, join("",
                    &zip([@stack_to_chr{@$stacks}], [@card_str_to_chr{@$cards}])
                )
            );
        }
    }
    else
    {
        die "File $fn does not start well!";
    }
}
