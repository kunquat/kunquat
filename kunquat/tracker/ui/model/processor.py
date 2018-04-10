# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.kunquat.kunquat import get_default_value
from .procparams.addparams import AddParams
from .procparams.bitcrusherparams import BitcrusherParams
from .procparams.compressparams import CompressParams
from .procparams.delayparams import DelayParams
from .procparams.envgenparams import EnvgenParams
from .procparams.filterparams import FilterParams
from .procparams.forceparams import ForceParams
from .procparams.freeverbparams import FreeverbParams
from .procparams.gaincompparams import GainCompParams
from .procparams.ksparams import KsParams
from .procparams.looperparams import LooperParams
from .procparams.noiseparams import NoiseParams
from .procparams.padsynthparams import PadsynthParams
from .procparams.panningparams import PanningParams
from .procparams.pitchparams import PitchParams
from .procparams.rangemapparams import RangeMapParams
from .procparams.ringmodparams import RingmodParams
from .procparams.sampleparams import SampleParams
from .procparams.slopeparams import SlopeParams
from .procparams.streamparams import StreamParams
from .procparams.volumeparams import VolumeParams


_proc_classes = {
    'add':          AddParams,
    'bitcrusher':   BitcrusherParams,
    'compress':     CompressParams,
    'delay':        DelayParams,
    'envgen':       EnvgenParams,
    'filter':       FilterParams,
    'force':        ForceParams,
    'freeverb':     FreeverbParams,
    'gaincomp':     GainCompParams,
    'ks':           KsParams,
    'looper':       LooperParams,
    'noise':        NoiseParams,
    'padsynth':     PadsynthParams,
    'panning':      PanningParams,
    'pitch':        PitchParams,
    'rangemap':     RangeMapParams,
    'ringmod':      RingmodParams,
    'sample':       SampleParams,
    'slope':        SlopeParams,
    'stream':       StreamParams,
    'volume':       VolumeParams,
}


class Processor():

    @staticmethod
    def get_params_class(format_name):
        return _proc_classes.get(format_name, None)

    def __init__(self, au_id, proc_id):
        assert au_id
        assert proc_id
        assert proc_id.startswith(au_id)
        assert len(proc_id.split('/')) == len(au_id.split('/')) + 1
        self._au_id = au_id
        self._proc_id = proc_id
        self._store = None
        self._controller = None

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller

    def _get_key(self, subkey):
        return '{}/{}'.format(self._proc_id, subkey)

    def get_existence(self):
        key = self._get_key('p_manifest.json')
        manifest = self._store.get(key, None)
        return (type(manifest) == dict)

    def _get_ports(self, port_format):
        ports = []
        for i in range(0x100):
            port_id = port_format.format(i)
            key = self._get_key('{}/p_manifest.json'.format(port_id))
            if key in self._store:
                ports.append(port_id)

        return ports

    def get_in_ports(self):
        return self._get_ports('in_{:02x}')

    def get_out_ports(self):
        return self._get_ports('out_{:02x}')

    def get_name(self):
        key = self._get_key('m_name.json')
        return self._store.get(key)

    def set_name(self, name):
        key = self._get_key('m_name.json')
        self._store[key] = name

    def get_message(self):
        key = self._get_key('m_message.json')
        return self._store.get(key, '')

    def set_message(self, message):
        key = self._get_key('m_message.json')
        self._store[key] = message or None

    def get_type(self):
        key = self._get_key('p_manifest.json')
        manifest = self._store.get(key)
        if manifest:
            return manifest['type']
        return None

    def get_type_params(self):
        if self.get_type() not in _proc_classes:
            return None

        cons = _proc_classes[self.get_type()]
        return cons(self._proc_id, self._controller)

    def get_signal_type(self):
        key = self._get_key('p_signal_type.json')
        return self._store.get(key, get_default_value(key))

    def set_signal_type(self, signal_type):
        key = self._get_key('p_signal_type.json')
        self._store[key] = signal_type

    def get_port_info(self):
        type_params = self.get_type_params()
        if not type_params:
            return {}

        return type_params.get_port_info()


