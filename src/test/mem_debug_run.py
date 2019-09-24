#!/usr/bin/env python3
# coding=utf-8

#
# Author: Tomi Jylhä-Ollila, Finland 2012-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import itertools as it
import os.path
import queue
import re
import shlex
import subprocess
import sys
import threading
import time


def progress_str(bar_width, area_width, offset):
    assert bar_width >= 1
    assert area_width % 2 == 0
    assert offset <= area_width - bar_width

    trailing_space = area_width - bar_width - offset
    assert offset + bar_width + trailing_space == area_width

    leading  = it.repeat(0, offset)
    bar      = it.repeat(1, bar_width)
    trailing = it.repeat(0, trailing_space)
    nums     = list(it.chain(leading, bar, trailing))

    compressed = (a + 2 * b for (a, b) in
            zip(it.islice(nums, 0, None, 2), it.islice(nums, 1, None, 2)))
    block_map = ['⣉', '⣏', '⣹', '⣿']
    return '⢸' + ''.join([block_map[x] for x in compressed]) + '⡇'


def show_beat(q, test_name, bar_width, area_width):
    offset_bound = area_width - bar_width + 1
    bars_fwd = [progress_str(bar_width, area_width, i)
            for i in range(offset_bound)]
    bars = bars_fwd + bars_fwd[-2:0:-1]
    beat = it.cycle(bars)
    info = 'Running test: {} '.format(test_name)
    line = info + next(beat)
    line_len = len(line)
    while q.empty():
        print(line, end='\r')
        line = info + next(beat)
        sys.stdout.flush()
        time.sleep(0.1)
    print(' ' * line_len, end='\r')


def strip_success_valgrind(output):
    lines = [line for line in output.splitlines()]
    vg_lines = (line for line in lines if line.startswith('=='))
    contains_address = re.compile('.*0x[0-9A-F]+:.*')
    if not any(bool(contains_address.match(vl)) for vl in vg_lines):
        lines = (line for line in lines if not line.startswith('=='))
    return '\n'.join(lines)


def run_test(program):
    q = queue.Queue()
    name = os.path.basename(program)
    beater = threading.Thread(target=show_beat, args=(q, name, 4, 10))

    command = 'valgrind --leak-check=full --show-leak-kinds=all ' + program
    try:
        beater.start()
        output = str(
                subprocess.check_output(shlex.split(command), stderr=subprocess.STDOUT),
                encoding='utf-8')
    except subprocess.CalledProcessError as e:
        q.put('done')
        beater.join()
        print(strip_success_valgrind(str(e.output, encoding='utf-8')))
        sys.exit(e.returncode)
    finally:
        q.put('done')
        beater.join()

    leaks = not 'no leaks are possible' in output

    errors = False
    for line in reversed(output.splitlines()):
        if 'ERROR SUMMARY:' in line:
            if ' 0 errors' not in line:
                errors = True
            break

    if leaks or errors:
        print(output)
        sys.exit(2)
    else:
        print(strip_success_valgrind(output))


if __name__ == '__main__':
    run_test(sys.argv[1])


