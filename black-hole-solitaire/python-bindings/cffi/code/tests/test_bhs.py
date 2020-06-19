#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
test_bhs
----------------------------------

Tests for `bhs` module.
"""

import pytest  # noqa: F401


def test_bhs():
    """Sample pytest test function with the pytest fixture as an argument.
    """
    import black_hole_solver
    solver = black_hole_solver.BlackHoleSolver()
    assert solver
    import pysol_cards.cards
    from pysol_cards.deal_game import Game
    from pysol_cards.random_base import RandomBase
    solver.read_board(
        Game('black_hole', 5, RandomBase.DEALS_MS).calc_deal_string(
            5, pysol_cards.cards.CardRenderer(print_ts=True)))
    assert solver.resume_solution() == 0
