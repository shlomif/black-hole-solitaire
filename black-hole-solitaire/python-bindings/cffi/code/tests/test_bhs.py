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
    b = black_hole_solver.BlackHoleSolver()
    assert b
