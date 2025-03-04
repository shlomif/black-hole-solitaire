#! /usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:fenc=utf-8
#
# Copyright © 2020 Shlomi Fish <shlomif@cpan.org>
#
# Distributed under terms of the MIT license.

from pydistman import DistManager

dist_name = "black_hole_solver"

obj = DistManager(
    aur_email="shlomif@cpan.org",
    dist_name=dist_name,
    dist_version="0.2.4",
    entry_point="none",
    full_name="Shlomi Fish",
    github_username="shlomif",
    project_email="shlomif@cpan.org",
    project_name="Black Hole Solver Wrapper",
    project_short_description="Solve Golf and Black Hole solitaires",
    project_year="2020",
    release_date="2025-03-04",
)
obj.cli_run()
