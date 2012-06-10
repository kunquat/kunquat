#!/usr/bin/env python
# coding=utf-8

#
# Author: Tomi Jylhä-Ollila, Finland 2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import print_function
import shlex
import subprocess
import sys


def strip_valgrind(output):
    lines = (line for line in output.splitlines()
             if not line.startswith('=='))
    return '\n'.join(lines)


def run_test(program):
    command = 'valgrind --leak-check=full --show-reachable=yes ' + program
    try:
        output = subprocess.check_output(
                shlex.split(command),
                stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        print(strip_valgrind(e.output))
        sys.exit(e.returncode)

    leaks = not 'no leaks are possible' in output

    if leaks:
        print(output)
        sys.exit(2)
    else:
        print(strip_valgrind(output))


if __name__ == '__main__':
    run_test(sys.argv[1])


