#! /usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:fenc=utf-8
#
# Copyright © 2021 Shlomi Fish < https://www.shlomifish.org/ >
#
# Licensed under the terms of the MIT license.

"""

"""

import yaml


def main():
    """docstring for main"""
    with open("./.travis.yml", "rt") as infh:
        data = yaml.safe_load(infh)
        steps = []
        steps.append({"run": ("sudo apt-get update -qq"), })
        steps.append({
            "run": ("sudo apt-get --no-install-recommends install " +
                    " ".join(data['addons']['apt']['packages'])
                    ),
            }
        )
        for arr in ['before_install', 'install', 'script']:
            steps += [{"run": x} for x in data[arr]]
        o = {'jobs': {'test-black-hole-solver': {'runs-on': 'ubuntu-latest',
             'steps': steps, }},
             'name': 'use-github-actions', 'on': ['push', ], }
        with open(".github/workflows/use-github-actions.yml", "wt") as outfh:
            # yaml.safe_dump(o, outfh)
            yaml.safe_dump(o, stream=outfh, canonical=False, indent=4, )


main()
