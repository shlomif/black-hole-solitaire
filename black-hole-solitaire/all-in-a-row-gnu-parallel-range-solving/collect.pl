#!/usr/bin/perl

use strict;
use warnings;
use autodie;

use IO::All;

# Threshold in seconds.
my $THRESHOLD  = 5;
my $SUMMARY_FN = "SUMMARY.txt";

sub sum_fh
{
    return io->file($SUMMARY_FN);
}

my $time = time() - $THRESHOLD;

my $start_idx;

if ( -e $SUMMARY_FN )
{
    my $last_line = `tail -1 "$SUMMARY_FN"`;
    ($start_idx) = $last_line =~ m/\A(\d+)/;
    $start_idx++;
}
else
{
    $start_idx = 1;
}

my $idx = $start_idx;

sub fn
{
    return "summaries/$idx.summary";
}

sub fh
{
    return io->file( fn() );
}

my $sum_fh = sum_fh();

while ( ( -e fn() ) && ( fh()->mtime() <= $time ) )
{
    $sum_fh->append( fh()->all() );
    fh()->unlink();
}
continue
{
    $idx++;
}
