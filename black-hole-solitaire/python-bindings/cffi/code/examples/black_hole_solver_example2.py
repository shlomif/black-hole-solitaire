#! /usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:fenc=utf-8
#
# Copyright © 2020 Shlomi Fish <shlomif@cpan.org>
#
# Distributed under the terms of the MIT license.
#
# This program is a black_hole_solver demo

from black_hole_solver import BlackHoleSolver

from pysol_cards.cards import CardRenderer
from pysol_cards.deal_game import Game
from pysol_cards.random_base import RandomBase
# from pysol_cards.single_deal_args_parse import SingleDealArgsParser


def make_pysol_board__main(game_num):
    return Game(
        "black_hole", game_num,
        RandomBase.DEALS_PYSOLFC,
    ).calc_layout_string(CardRenderer(True))


def main():
    solver = BlackHoleSolver()
    idx = 1
    while True:
        pysolfc_black_hole_deal = make_pysol_board__main(idx)
        solver.read_board(
            board=pysolfc_black_hole_deal,
            game_type='black_hole',
            place_queens_on_kings=True,
            wrap_ranks=True)
        solver.resume_solution()
        solver.recycle()
        idx += 1
        print('reached {}'.format(idx))


main()
