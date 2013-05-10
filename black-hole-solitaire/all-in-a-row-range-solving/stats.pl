#!/usr/bin/perl

use strict;
use warnings;

use Statistics::Descriptive;

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

my $unsolved_stats = Statistics::Descriptive::Full->new();
my $solved_stats_checked = Statistics::Descriptive::Full->new();
my $solved_stats_gen = Statistics::Descriptive::Full->new();

foreach my $n (1 .. 1_000_000)
{
    if ($n % 1_000 == 0)
    {
        print STDERR "Processing $n\n";
    }

    my $fn = "$n.rs";

    my $text = _slurp($fn);

    if ($text =~ m{\AUnsolved!})
    {
        if ($text !~ m{^Total number of states checked is (\d+)\.\nThis scan generated \1 states\.$}ms)
        {
            die "Mismatching numbers in $fn.";            
        }
        else
        {
            $unsolved_stats->add_data($1);
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
            $solved_stats_checked->add_data($checked);
            $solved_stats_gen->add_data($gen);
        }
    }
}

foreach my $spec
(
    {
        title => "Unsolved",
        obj => $unsolved_stats,
    },
    {
        title => "Solved (Checked)",
        obj => $solved_stats_checked,
    },
    {
        title => "Solved (Generated)",
        obj => $solved_stats_gen,
    },
)
{
    my ($title, $stats) = @{$spec}{qw(title obj)};
    print "$title\n";
    print "---------------------------\n";
    print "Count: " , $stats->count(), "\n";
    print "Min: " , $stats->min(), "\n";
    print "Max: " , $stats->max(), "\n";
    print "Average: " , $stats->mean(), "\n";
    print "StdDev: " , $stats->standard_deviation(), "\n";
    print "Median: " , $stats->median(), "\n";
    print "\n";
}

