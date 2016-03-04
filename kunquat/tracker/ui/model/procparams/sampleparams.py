# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from procparams import ProcParams


class SampleParams(ProcParams):

    _SAMPLES_MAX = 512

    @staticmethod
    def get_default_signal_type():
        return u'voice'

    @staticmethod
    def get_port_info():
        return {
            'in_00':  u'pitch',
            'in_01':  u'force',
            'out_00': u'audio L',
            'out_01': u'audio R',
        }

    def __init__(self, proc_id, controller):
        ProcParams.__init__(self, proc_id, controller)

    def _get_sample_id(self, sample_num):
        return 'smp_{:03x}'.format(sample_num)

    def _get_sample_key(self, sample_id, key):
        return '{}/{}'.format(sample_id, key)

    def _get_sample_header(self, sample_id):
        sample_key = self._get_sample_key(sample_id, 'p_sh_sample.json')
        header = self._get_value(sample_key, None)
        return header

    def _set_sample_header(self, sample_id, header):
        self._set_value(self._get_sample_key(sample_id, 'p_sh_sample.json'), header)

    def get_max_sample_count(self):
        return self._SAMPLES_MAX

    def get_sample_ids(self):
        ret_ids = []
        for i in xrange(self._SAMPLES_MAX):
            cur_id = self._get_sample_id(i)
            cur_header = self._get_sample_header(cur_id)
            if type(cur_header) == dict:
                ret_ids.append(cur_id)

        return ret_ids

    def get_selected_sample_id(self):
        return self._session.get_selected_sample_id(self._proc_id)

    def set_selected_sample_id(self, sample_id):
        self._session.set_selected_sample_id(self._proc_id, sample_id)

    def get_sample_name(self, sample_id):
        return self._get_value(self._get_sample_key(sample_id, 'm_name.json'), None)

    def set_sample_name(self, sample_id, name):
        self._set_value(self._get_sample_key(sample_id, 'm_name.json'), name)

    def get_sample_freq(self, sample_id):
        return self._get_sample_header(sample_id)['freq']

    def set_sample_freq(self, sample_id, freq):
        header = self._get_sample_header(sample_id)
        header['freq'] = freq
        self._set_sample_header(sample_id, header)


