# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

'''Simple utility for finding the amount of physical CPU cores.
'''


import math
import multiprocessing
import sys


def get_core_count():
    if sys.platform.startswith('linux'):
        cpuinfo = None
        try:
            with open('/proc/cpuinfo') as f:
                cpuinfo = f.read(1048576)
        except:
            pass

        if cpuinfo:
            procs = [p for p in cpuinfo.split('\n\n') if p]
            cores = set()
            for p in procs:
                id_infos = [i for i in p.split('\n')
                        if i.startswith(('physical id', 'core id'))]
                id_infos.sort()
                core_ids = tuple(int(i.split(' ')[-1]) for i in id_infos)
                cores.add(core_ids)

            cpu_count = len(cores)
            if cpu_count > 1:
                return cpu_count

    try:
        cpu_count = multiprocessing.cpu_count()
    except NotImplementedError:
        cpu_count = 1

    if cpu_count <= 2:
        return max(1, cpu_count)
    return min(max(2, int(math.ceil(cpu_count / 2))), 32)


