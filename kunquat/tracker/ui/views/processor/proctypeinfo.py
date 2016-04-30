# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from .addproc import AddProc
from .delayproc import DelayProc
from .envgenproc import EnvgenProc
from .filterproc import FilterProc
from .forceproc import ForceProc
from .freeverbproc import FreeverbProc
from .gaincomp_proc import GainCompProc
from .padsynthproc import PadsynthProc
from .panningproc import PanningProc
from .pitchproc import PitchProc
from .ringmodproc import RingmodProc
from .sampleproc import SampleProc
from .streamproc import StreamProc
from .volumeproc import VolumeProc
from .unsupportedproc import UnsupportedProc


_proc_classes = {
    'add':      AddProc,
    'delay':    DelayProc,
    'envgen':   EnvgenProc,
    'filter':   FilterProc,
    'force':    ForceProc,
    'freeverb': FreeverbProc,
    'gaincomp': GainCompProc,
    'padsynth': PadsynthProc,
    'panning':  PanningProc,
    'pitch':    PitchProc,
    'ringmod':  RingmodProc,
    'sample':   SampleProc,
    'stream':   StreamProc,
    'volume':   VolumeProc,
}


def get_class(proc_type):
    return _proc_classes.get(proc_type, UnsupportedProc)


def get_sorted_type_info_list():
    return sorted(list(_proc_classes.items()), key=lambda x: x[1].get_name())


def get_supported_classes():
    ls = get_sorted_type_info_list()
    return [cls for (name, cls) in ls]


