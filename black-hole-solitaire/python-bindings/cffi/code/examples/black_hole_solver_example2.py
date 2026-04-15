#! /usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:fenc=utf-8
#
# Copyright © 2023 Shlomi Fish <shlomif@cpan.org>
#
# Distributed under the terms of the MIT license.
#
# This program is a black_hole_solver demo
'''
A range-solver to test memory-consumption .
'''

import sys

from black_hole_solver import BlackHoleSolver

from pysol_cards.cards import CardRenderer
from pysol_cards.deal_game import Game
from pysol_cards.random_base import RandomBase
# from pysol_cards.single_deal_args_parse import SingleDealArgsParser

VARIANT = "black_hole"
renderer = CardRenderer(True)


def make_pysol_board(deal_idx):
    return Game(
        VARIANT, deal_idx,
        RandomBase.DEALS_PYSOLFC,
    ).calc_layout_string(renderer)


def main():
    solver = BlackHoleSolver()
    deal_idx = 0
    max_num_times = -1
    longest_idx = -1

    def _output_progress():
        print('Reached deal No. {} [ '
              'max_num_times = {} ; longest_idx = {} ]'.format(
                  deal_idx, max_num_times, longest_idx), flush=True)
    while True:
        if len(sys.argv) <= 1:
            break
        arg = sys.argv.pop(1)
        if arg == 'seq':
            arg = sys.argv.pop(1)
            start = int(arg)
            arg = sys.argv.pop(1)
            top = int(arg)
            r = range(start, top + 1)
        else:
            start = int(arg)
            r = range(start, start + 1)

        for deal_idx in r:
            if deal_idx % 20 == 0:
                _output_progress()
            board = make_pysol_board(deal_idx)
            solver.read_board(
                board=board,
                game_type=VARIANT,
                place_queens_on_kings=True,
                wrap_ranks=True,
            )
            solver.resume_solution()
            this_count = solver.get_num_times()
            if max_num_times < this_count:
                max_num_times = this_count + 0
                longest_idx = deal_idx
                _output_progress()
            solver.recycle()


if __name__ == "__main__":
    main()
