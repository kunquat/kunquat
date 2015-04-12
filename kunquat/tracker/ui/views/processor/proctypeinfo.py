# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from addproc import AddProc
from chorusproc import ChorusProc
from delayproc import DelayProc
from envgenproc import EnvgenProc
from freeverbproc import FreeverbProc
from ringmodproc import RingmodProc
from sampleproc import SampleProc
from volumeproc import VolumeProc
from unsupportedproc import UnsupportedProc


_proc_classes = {
    'add':      AddProc,
    'chorus':   ChorusProc,
    'delay':    DelayProc,
    'envgen':   EnvgenProc,
    'freeverb': FreeverbProc,
    'ringmod':  RingmodProc,
    'volume':   VolumeProc,
}


def get_class(proc_type):
    return _proc_classes.get(proc_type, UnsupportedProc)


def get_sorted_type_info_list():
    return sorted(list(_proc_classes.items()), key=lambda x: x[1].get_name())


def get_supported_classes():
    ls = get_sorted_type_info_list()
    return [cls for (name, cls) in ls]


