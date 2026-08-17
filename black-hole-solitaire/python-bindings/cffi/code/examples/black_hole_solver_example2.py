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


class IntStats:
    """docstring for IntStats"""
    def __init__(self):
        self.n = 0
        self.s = 0
        self.sq = 0
        self.max_val = -1
        self.max_key = -1

    def add(self, key, val):
        """docstring for add"""
        assert isinstance(val, int)
        self.n += 1
        self.s += val
        self.sq += pow(val, 2)
        if val > self.max_val:
            self.max_val = val
            self.max_key = key
            return True
        return False


BLACK_HOLE_SOLVER__NOT_SOLVABLE = 8


def main():
    solver = BlackHoleSolver()
    deal_idx = 0
    stats = {False: IntStats(), True: IntStats(), }
    states_stats = {False: IntStats(), True: IntStats(), }

    def _output_progress():
        print('Reached deal No. {} [ '
              'max_num_times = {};{} ; longest_idx = {};{} ]'.format(
                  deal_idx, stats[False].max_val, stats[True].max_val,
                  stats[False].max_key, stats[True].max_key,
                  ), flush=True)
        print('Reached deal No. {} [ '
              'max_num_states = {};{} ; longest_idx = {};{} ]'.format(
                  deal_idx, states_stats[False].max_val,
                  states_stats[True].max_val,
                  states_stats[False].max_key, states_stats[True].max_key,
                  ), flush=True)
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
            ret_code = solver.resume_solution()
            if ret_code == solver.BLACK_HOLE_SOLVER__SUCCESS:
                was_solved = True
            elif ret_code == BLACK_HOLE_SOLVER__NOT_SOLVABLE:
                was_solved = False
            else:
                raise Exception("mis handled ret_code")
            this_count = solver.get_num_times()
            if stats[was_solved].add(deal_idx, this_count):
                _output_progress()
            this_count = solver.get_num_states_in_collection()
            if states_stats[was_solved].add(deal_idx, this_count):
                _output_progress()
            solver.recycle()


if __name__ == "__main__":
    main()
