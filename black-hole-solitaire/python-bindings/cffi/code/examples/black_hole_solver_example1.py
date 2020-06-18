#! /usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:fenc=utf-8
#
# Copyright © 2020 Shlomi Fish <shlomif@cpan.org>
#
# Distributed under the terms of the MIT license.
#
# This program displays increasing sums of two positive integers

from six import print_

from sum_walker import DWIM_SumWalker


def main():
    seq = [1, 2, 3, 4]

    def request_more():
        nonlocal seq
        seq.append(seq[-1] + 1)

    it = DWIM_SumWalker(2, seq, request_more)

    def print_next():
        nonlocal it
        sum_, coords = next(it)
        print_("{} = {}".format(
            sum_, " ; ".join(
                [" + ".join([str(seq[x]) for x in permutation])
                 for permutation in coords])))

    # Prints «2 = 1 + 1»
    print_next()

    # Prints «3 = 1 + 2»
    print_next()

    # Prints «4 = 1 + 3 ; 2 + 2»
    print_next()

    # Prints «5 = 1 + 4 ; 2 + 3»
    print_next()

    # Prints «6 = 1 + 5 ; 2 + 4 ; 3 + 3»
    print_next()





main()
