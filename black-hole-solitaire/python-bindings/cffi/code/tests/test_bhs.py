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
    deal_idx = 5
    solver.read_board(
        board=Game(
            'black_hole', deal_idx, RandomBase.DEALS_MS
        ).calc_deal_string(
            deal_idx, pysol_cards.cards.CardRenderer(print_ts=True)),
        game_type='black_hole',
        place_queens_on_kings=True,
        wrap_ranks=True,
    )
    assert solver.resume_solution() == 0


def test_limit_iters():
    import black_hole_solver
    solver = black_hole_solver.BlackHoleSolver()
    assert solver
    import pysol_cards.cards
    from pysol_cards.deal_game import Game
    from pysol_cards.random_base import RandomBase
    for deal_idx in [5, 4]:
        solver.read_board(
            board=Game(
                'black_hole', deal_idx, RandomBase.DEALS_MS
            ).calc_deal_string(
                deal_idx, pysol_cards.cards.CardRenderer(print_ts=True)),
            game_type='black_hole',
            place_queens_on_kings=True,
            wrap_ranks=True,
        )
        solver.limit_iterations(200 * 1000)
        assert solver.resume_solution() == 0
        assert solver.get_num_times() > 0
        assert solver.get_num_states_in_collection() > 0
        move = solver.get_next_move()
        assert 0 <= move.get_column_idx() < 17
        solver.recycle()
    for deal_idx in [5]:
        solver = black_hole_solver.BlackHoleSolver()
        solver.read_board(
            board=Game(
                'black_hole', deal_idx, RandomBase.DEALS_MS
            ).calc_deal_string(
                deal_idx, pysol_cards.cards.CardRenderer(print_ts=True)),
            game_type='black_hole',
            place_queens_on_kings=True,
            wrap_ranks=True,
        )
        limit = 20
        solver.limit_iterations(limit)
        assert solver.resume_solution() == \
            black_hole_solver.BlackHoleSolver.BLACK_HOLE_SOLVER__OUT_OF_ITERS
        assert solver.get_num_times() == limit
        assert solver.get_num_states_in_collection() > 0
        assert solver.resume_solution() == \
            black_hole_solver.BlackHoleSolver.BLACK_HOLE_SOLVER__OUT_OF_ITERS
        assert solver.get_num_times() == limit
        assert solver.get_num_states_in_collection() > 0
        solver.limit_iterations(limit)
        assert solver.resume_solution() == \
            black_hole_solver.BlackHoleSolver.BLACK_HOLE_SOLVER__OUT_OF_ITERS
        assert solver.get_num_times() == limit
        assert solver.get_num_states_in_collection() > 0
        solver.recycle()
