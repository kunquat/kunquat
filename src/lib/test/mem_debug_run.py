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
import itertools as it
import os.path
import Queue
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
            it.izip(it.islice(nums, 0, None, 2), it.islice(nums, 1, None, 2)))
    block_map = [u' ', u'⡇', u'⢸', u'⣿']
    return u'⢾' + u''.join([block_map[x] for x in compressed]) + u'⡷'


def show_beat(q, test_name, bar_width, area_width):
    offset_bound = area_width - bar_width + 1
    bars_fwd = [progress_str(bar_width, area_width, i)
            for i in xrange(offset_bound)]
    bars = bars_fwd + bars_fwd[-2:0:-1]
    beat = it.cycle(bars)
    info = u'Running test: {0} '.format(test_name)
    line = info + beat.next()
    line_len = len(line)
    while q.empty():
        print(line.encode('utf-8'), end='\r')
        line = info + beat.next()
        sys.stdout.flush()
        time.sleep(0.1)
    print(' ' * line_len, end='\r')


def strip_valgrind(output):
    lines = (line for line in output.splitlines()
             if not line.startswith('=='))
    return '\n'.join(lines)


def run_test(program):
    q = Queue.Queue()
    name = os.path.basename(program)
    beater = threading.Thread(target=show_beat, args=(q, name, 4, 10))

    command = 'valgrind --leak-check=full --show-reachable=yes ' + program
    try:
        beater.start()
        output = subprocess.check_output(
                shlex.split(command),
                stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        q.put('done')
        beater.join()
        print(strip_valgrind(e.output))
        sys.exit(e.returncode)
    finally:
        q.put('done')
        beater.join()

    leaks = not 'no leaks are possible' in output

    if leaks:
        print(output)
        sys.exit(2)
    else:
        print(strip_valgrind(output))


if __name__ == '__main__':
    run_test(sys.argv[1])


