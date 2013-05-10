#!/usr/bin/perl

use strict;
use warnings;
use autodie;

use IO::All;

# Threshold in seconds.
my $THRESHOLD = 5;
my $SUMMARY_FN = "SUMMARY.txt";

my $time = time() - $THRESHOLD;

my $start_idx;

if (my $last_line = io($SUMMARY_FN)->[-1])
{
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
    return io->file(fn());
}

while ((-e fn()) && (fh()->mtime() <= $time))
{
    fh() >> io($SUMMARY_FN);
    fh()->unlink();
}
continue
{
    $idx++;
}
