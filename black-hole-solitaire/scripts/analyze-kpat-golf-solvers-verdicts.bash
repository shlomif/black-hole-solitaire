#!/usr/bin/env bash
#
# analyze.bash
# Copyright (C) 2019 Shlomi Fish <shlomif@cpan.org>
#
# Distributed under terms of the Expat license.
#


filt2()
{
    grep -E '^[1-9]' | head -20000
}
filt()
{
    filt2 | perl -lanE 'say $F[1]'
}
head_()
{
    perl -lE '$_="Old\tNew\tCount";say;s/\w/-/g;say;'
}
(
    head_
    paste <(< old-golfs.txt filt) <(< new-golfs.txt filt) | sort | uniq -c | perl -lanE 'say "$F[1]\t$F[2]\t$F[0]"'
) | perl -lanE 'printf"%-20s%-20s%s\n",@F'
