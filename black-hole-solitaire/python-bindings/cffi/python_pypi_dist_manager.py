#! /usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:fenc=utf-8
#
# Copyright © 2020 Shlomi Fish <shlomif@cpan.org>
#
# Distributed under terms of the MIT license.

from pydistman import DistManager


class MyDistManager(DistManager):
    def _build_only_command_custom_steps(self):
        for fn in ["{dest_dir}/PKGBUILD", "{dest_dir}/PKGBUILD-git",
                   "{dest_dir}/setup.py", ]:
            s = "https://github.com/shlomif/black-hole-solitaire"
            self._re_mutate(
                fn_proto=fn,
                pattern=("https://github\\.com/shlomif/black_hole_solver"),
                prefix='',
                suffix=s,
            )


dist_name = "black_hole_solver"

obj = MyDistManager(
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
