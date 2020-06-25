#! /usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:fenc=utf-8
#
# Copyright © 2020 Shlomi Fish <shlomif@cpan.org>
#
# Distributed under terms of the MIT license.

import os
import os.path
import re
import shutil
import sys
from subprocess import check_call

import cookiecutter.main


class DistGenerator(object):
    """docstring for DistGenerator"""
    def __init__(self, dist_name, base_dir):
        self.dist_name = dist_name
        self.base_dir = base_dir
        self.src_dir = "code"
        self.src_modules_dir = self.src_dir + "/" + dist_name
        self.dest_dir = 'dest'
        self.dest_modules_dir = self.dest_dir + "/" + dist_name

    def _slurp(self, fn):
        return open(fn, "rt").read()

    def _fmt_slurp(self, fn_proto):
        return self._slurp(self._myformat(fn_proto))

    def _myformat(self, mystring):
        return mystring.format(
            base_dir=self.base_dir,
            dest_dir=self.dest_dir,
            dest_modules_dir=self.dest_modules_dir,
            dist_name=self.dist_name,
            src_dir=self.src_dir,
            src_modules_dir=self.src_modules_dir,
        )

    def command__build(self):
        self.command__build_only()
        self.command__test()

    def command__build_only(self):
        if os.path.exists(self.dist_name):
            shutil.rmtree(self.dist_name)
        cookiecutter.main.cookiecutter(
            'gh:Kwpolska/python-project-template',
            no_input=True,
            overwrite_if_exists=True,
            extra_context={
                "entry_point": ["none", "cli", "gui", ],
                "project_name": "Black Hole Solver Wrapper",
                "project_short_description": "lib",
                "release_date": "2020-06-23",
                "repo_name": self.dist_name,
                "version": "0.2.3",
                "year": "2020",
                'aur_email': "shlomif@cpan.org",
                'email': "shlomif@cpan.org",
                'full_name': 'Shlomi Fish',
                'github_username': "shlomif",
                },
            )
        os.rename(self.dist_name, self.dest_dir)

        def _append(to_proto, from_, make_exe=False):
            to = self._myformat(to_proto)
            open(to, "at").write(
                self._fmt_slurp(from_))
            if make_exe:
                os.chmod(to, 0o755)

        _append("{dest_modules_dir}/__init__.py",
                "{src_modules_dir}/__init__.py")

        def _re_mutate(fn_proto, pattern, repl_fn_proto, prefix='', suffix=''):
            fn = self._myformat(fn_proto)
            replacement_string = \
                (prefix +
                 self._fmt_slurp(repl_fn_proto) +
                 suffix)
            txt = self._slurp(fn)
            txt, count = re.subn(
                pattern,
                replacement_string.replace('\\', '\\\\'),
                txt,
                1,
                re.M | re.S
            )
            # print(count)
            assert count == 1
            open(fn, "wt").write(txt)
        _re_mutate(
            "{dest_dir}/CHANGELOG.rst",
            "\n0\\.1\\.0\n.*",
            "{src_dir}/CHANGELOG.rst.base.txt", "\n")
        s = "COPYRIGHT\n"
        for fn in ["{dest_dir}/README", "{dest_dir}/README.rst",
                   "{dest_dir}/docs/README.rst", ]:
            _re_mutate(
                fn, "^PURPOSE\n.*?\n" + s, "{src_dir}/README.part.rst", '', s)

        req_fn = "{src_dir}/requirements.txt"
        _append("{dest_dir}/requirements.txt",
                req_fn)
        _append("{dest_dir}/tests/test_bhs.py",
                "{src_dir}/tests/test_bhs.py",
                make_exe=True)
        open(self._myformat("{dest_dir}/tox.ini"), "wt").write(
            "[tox]\nenvlist = py38\n\n" +
            "[testenv]\ndeps =" + "".join(
                ["\n\t" + x for x in
                 self._fmt_slurp(req_fn).split("\n")]) + "\n" +
            "\ncommands = pytest\n")

    def command__test(self):
        check_call(["bash", "-c", self._myformat("cd {dest_dir} && tox")])

    def command__release(self):
        self.command__build()
        check_call(["bash", "-c", self._myformat(
            "cd {dest_dir} && python3 setup.py sdist " +
            " && twine upload --verbose dist/{dist_name}*.tar.gz")])

    def command__gen_travis_yaml(self):
        import yaml

        with open("travis.yml", "wt") as f:
            f.write(yaml.dump({
                'install':
                [
                    'pip install cookiecutter',
                    self._myformat(
                        '( cd {base_dir} && ' +
                        'python3 wrapper.py build_only )'),
                    self._myformat(
                        '( cd {base_dir} && cd {dest_dir} && ' +
                        'pip install -r requirements.txt && pip install . )')
                ],
                'script': [
                    self._myformat(
                        '( cd {base_dir} && cd {dest_dir} && ' +
                        'py.test --cov {dist_name} ' +
                        '--cov-report term-missing tests/ )')
                ],
                'language': 'python',
                'python': ['3.5', '3.6', '3.7', '3.8', 'pypy3', ],
                }))

    def run_command(self, cmd, args):
        if cmd == 'travis':
            obj.command__gen_travis_yaml()
        elif cmd == 'build':
            obj.command__build()
        elif cmd == 'build_only':
            obj.command__build_only()
        elif cmd == 'release':
            obj.command__release()
        else:
            raise BaseException("Unknown sub-command")


try:
    cmd = sys.argv.pop(1)
except IndexError:
    cmd = 'build'

dist_name = "black_hole_solver"
base_dir = "python-" + dist_name

obj = DistGenerator(dist_name=dist_name, base_dir=base_dir)
obj.run_command(cmd=cmd, args=[])
