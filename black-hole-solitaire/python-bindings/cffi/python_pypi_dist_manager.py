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
    dist_name=dist_name,
    dist_version="0.2.3",
    project_name="Black Hole Solver Wrapper",
    project_short_description="Solve Golf and Black Hole solitaires",
    release_date="2020-06-23",
    project_year="2020",
    aur_email="shlomif@cpan.org",
    project_email="shlomif@cpan.org",
    full_name="Shlomi Fish",
    github_username="shlomif",
)
obj.cli_run()
