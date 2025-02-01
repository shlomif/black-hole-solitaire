#! /usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:fenc=utf-8
#
# Copyright © 2021 Shlomi Fish < https://www.shlomifish.org/ >
#
# Licensed under the terms of the MIT license.

"""

"""

import re
import yaml


def _write(output_path, data):
    with open(output_path, "wt") as outfh:
        # yaml.safe_dump(o, outfh)
        outfh.write(
            "# This file is GENERATED BY\n" +
            "# black-hole-solitaire/CI-testing/" +
            "translate-travis.yml-to-github-actions.py\n"
        )
        yaml.safe_dump(data, stream=outfh, canonical=False, indent=4, )


def _apply_cpack_fix(steps):
    # See:
    #
    # https://github.com/sithlord48/blackchocobo/actions/runs/1207252836/workflow#L100
    # https://github.community/t/windows-actions-suddenly-fail-needing-a-nuspec-file/199483
    cpack_fix = {
        "if": "runner.os == 'Windows'",
        "name": "Remove Chocolatey's CPack",
        "run": "Remove-Item -Path " +
        "C:\\ProgramData\\Chocolatey\\bin\\cpack.exe -Force",
    }
    cpack_fix_needed = False
    if cpack_fix_needed:
        steps.append(cpack_fix)


def main():
    generate_linux_yaml()
    generate_windows_yaml(
        plat='x64',
        output_path=".github/workflows/windows-x64.yml",
        is_act=False,
    )


def generate_linux_yaml():
    """docstring for main"""
    with open("./.travis.yml", "rt") as infh:
        data = yaml.safe_load(infh)
    steps = []
    steps.append({
        "uses": ("actions/checkout@v2"),
        "with": {
            "submodules": "true",
        },
    })
    if 0:
        steps.append({
            "run":
            ("cd workflow ; (ls -lrtA ; false)"), })
    elif 0:
        steps.append({
            "run":
            ("cd . ; (ls -lrtA ; false)"), })
    steps.append({"run": ("sudo apt-get update -qq"), })
    steps.append({
        "run": ("sudo apt-get --no-install-recommends install -y " +
                " ".join(data['addons']['apt']['packages'])
                ),
        }
    )
    local_lib_shim = 'local_lib_shim() { eval "$(perl ' + \
        '-Mlocal::lib=$HOME/' + \
        'perl_modules)"; } ; local_lib_shim ; '
    for arr in ['before_install', 'install', 'script']:
        steps += [{"run": local_lib_shim + x} for x in data[arr]]
    o = {'jobs': {'test-black-hole-solver': {'runs-on': 'ubuntu-latest',
         'steps': steps, }},
         'name': 'use-github-actions', 'on': ['push', ], }
    output_path = ".github/workflows/use-github-actions.yml"
    _write(output_path=output_path, data=o, )


def generate_windows_yaml(plat, output_path, is_act):
    x86 = (plat == 'x86')
    with open("./.disabled-appveyor.yml", "rt") as infh:
        data = yaml.safe_load(infh)
    with open("./black-hole-solitaire/CI-testing/gh-actions--" +
              "windows-yml--from-p5-UV.yml", "rt") as infh:
        skel = yaml.safe_load(infh)
    steps = skel['jobs']['perl']['steps']
    while steps[-1]['name'] != 'perl -V':
        steps.pop()

    cpanm_step = {
        "name": "install cpanm and mult modules",
        "uses": "perl-actions/install-with-cpanm@v1",
    }
    steps.append(cpanm_step)
    if False:
        mingw = {
            "name": "Set up MinGW",
            "uses": "egor-tensin/setup-mingw@v2",
            "with": {
                "platform": plat,
            },
        }
        steps.append(mingw)

    _apply_cpack_fix(steps=steps,)

    def _calc_batch_code(cmds):
        batch = ""
        batch += "@echo on\n"
        for k, v in sorted(data['environment'].items()):
            # batch += "SET " + k + "=\"" + v + "\"\n"
            batch += "SET " + k + "=" + v + "\n"
        if x86:
            start = 'mkdir pkg-build-win64'
            end = "^cpack -G WIX"
        else:
            start = 'mkdir pkg-build'
            end = "^7z a"
        if False:
            start_idx = [
                i for i, cmd in enumerate(cmds)
                if cmd == "cd .." and
                cmds[i+1] == start
            ][0]
            end_idx = start_idx + 1
            while not re.search(end, cmds[end_idx]):
                end_idx += 1
            cmds = cmds[:start_idx] + cmds[(end_idx+1):]

        for cmd in cmds:
            if cmd.startswith("cpanm "):
                words = cmd.split(' ')[1:]
                dw = []
                for w in words:
                    if not w.startswith("-"):
                        dw.append(w)
                nonlocal cpanm_step
                cpanm_step['with'] = {"install": "\n".join(dw), }
                continue
            if re.search("copy.*?python\\.exe", cmd):
                continue
            if re.search("^(?:SET|set) PATH=.*?strawberry", cmd):
                continue
            if "choco install strawberryperl" not in cmd:
                if "mingw32-make" in cmd.lower():
                    continue
                if x86:
                    if cmd.startswith("perl ../source/run-tests.pl"):
                        continue
                    r = re.sub(
                        "--dbm=kaztree",
                        "--dbm=none",
                        cmd
                    )
                else:
                    r = cmd
                # See:
                # https://serverfault.com/questions/157173
                shim = ''
                if not (r.lower().startswith("set ") or ("|" in r)):
                    shim = " || ( echo Failed & exit /B 1 )"
                if not (r.lower().startswith("set perl")):
                    batch += r + shim + "\n"
        return batch

    steps.append({
        'name': "install and test_script code",
        "run": _calc_batch_code(
            cmds=(data['install']+data['test_script'])
        ),
        "shell": "cmd",
    })

    def _myfilt(path):
        is32 = ("\\pkg-build\\" in path)
        return (is32 if x86 else (not is32))
    if False:
        steps += [{
            'name': "upload build artifacts - " + art['name'],
            'uses': "actions/upload-artifact@v2",
            'with': art
        } for art in data['artifacts'] if _myfilt(art['path'])]
    skel['name'] = ("windows-x86" if x86 else 'windows-x64')
    skel['on'] = ['push']
    _write(output_path=output_path, data=skel, )


main()
