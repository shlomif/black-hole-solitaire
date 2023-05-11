#! /usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:fenc=utf-8
#
# Copyright © 2023 Shlomi Fish <shlomif@cpan.org>
#
# Distributed under the terms of the MIT license.
#
# This program is a black_hole_solver demo

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
    while True:
        deal_idx += 1
        print('Reached deal No. {} [ max_num_times = {} ]'.format(
            deal_idx, max_num_times), flush=True)
        board = make_pysol_board(deal_idx)
        solver.read_board(
            board=board,
            game_type=VARIANT,
            place_queens_on_kings=True,
            wrap_ranks=True,
        )
        solver.resume_solution()
        max_num_times = max(max_num_times, solver.get_num_times())
        solver.recycle()


if __name__ == "__main__":
    main()
