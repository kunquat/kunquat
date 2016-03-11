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

from kunquat.kunquat.kunquat import Kunquat, KunquatFormatError
from procparams import ProcParams


class SampleParams(ProcParams):

    _SAMPLES_MAX = 512
    _RANDOM_LIST_LENGTH_MAX = 8

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

    def _get_full_sample_key(self, sample_id, key):
        sample_key = self._get_sample_key(sample_id, key)
        return self._get_conf_key(sample_key)

    def _get_sample_num(self, sample_id):
        return int(sample_id.split('_')[1], 16)

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

    def get_free_sample_id(self):
        for i in xrange(self._SAMPLES_MAX):
            cur_id = self._get_sample_id(i)
            cur_header = self._get_sample_header(cur_id)
            if not type(cur_header) == dict:
                return cur_id
        return None

    def import_sample(self, sample_id, path):
        with open(path, 'rb') as f:
            sample_data = f.read()

        validator = WavPackValidator(sample_data)
        if not validator.is_data_valid():
            return False
        del validator

        sample_data_key = self._get_full_sample_key(sample_id, 'p_sample.wv')
        sample_header_key = self._get_full_sample_key(sample_id, 'p_sh_sample.json')

        header = { u'format': u'WavPack', u'freq': 48000, }

        transaction = {}
        transaction[sample_data_key] = sample_data
        transaction[sample_header_key] = header

        self._store.put(transaction)

        return True

    def remove_sample(self, sample_id):
        key_prefix = self._get_full_sample_key(sample_id, '')

        transaction = {}
        for key in (k for k in self._store.iterkeys() if k.startswith(key_prefix)):
            transaction[key] = None

        # Remove all references in random lists
        sample_num = self._get_sample_num(sample_id)
        note_map = self._get_note_map()
        for (point, random_list) in note_map:
            random_list[:] = [e for e in random_list if e[2] != sample_num]
        transaction[self._get_conf_key('p_nm_note_map.json')] = note_map

        self._store.put(transaction)

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

    def _get_note_map(self):
        return self._get_value('p_nm_note_map.json', [])

    def _set_note_map(self, note_map):
        self._set_value('p_nm_note_map.json', note_map)

    def get_note_map_points(self):
        note_map = self._get_note_map()
        return [coords for (coords, _) in note_map]

    def get_selected_note_map_point(self):
        return self._session.get_selected_sample_note_map_point(self._proc_id)

    def set_selected_note_map_point(self, point):
        self._session.set_selected_sample_note_map_point(self._proc_id, point)

    def add_note_map_point(self, coords):
        assert coords not in self.get_note_map_points()
        note_map = self._get_note_map()
        note_map.append([coords, []])
        self._set_note_map(note_map)

    def _get_note_map_point_index(self, coords):
        note_map = self._get_note_map()
        index = [i for i, entry in enumerate(note_map) if entry[0] == coords][0]
        return index

    def move_note_map_point(self, old_coords, new_coords):
        assert new_coords not in self.get_note_map_points()
        note_map = self._get_note_map()
        index = self._get_note_map_point_index(old_coords)
        note_map[index][0] = new_coords
        self._set_note_map(note_map)

    def remove_note_map_point(self, coords):
        note_map = self._get_note_map()
        index = self._get_note_map_point_index(coords)
        del note_map[index]
        self._set_note_map(note_map)

    def get_selected_hit_info(self):
        return self._session.get_selected_sample_hit_info(self._proc_id)

    def set_selected_hit_info(self, hit_info):
        self._session.set_selected_sample_hit_info(self._proc_id, hit_info)

    def _get_random_list(self, coords):
        note_map = self._get_note_map()
        random_list = [r for e, r in note_map if e == coords][0]
        return random_list

    def get_note_map_random_list_length(self, coords):
        random_list = self._get_random_list(coords)
        return len(random_list)

    def is_note_map_random_list_full(self, coords):
        random_list = self._get_random_list(coords)
        return len(random_list) >= self._RANDOM_LIST_LENGTH_MAX

    def add_note_map_random_list_entry(self, coords):
        assert not self.is_note_map_random_list_full(coords)
        note_map = self._get_note_map()
        point_index = self._get_note_map_point_index(coords)

        sample_ids = self.get_sample_ids()
        some_sample_num = self._get_sample_num(sample_ids[0]) if sample_ids else 0

        note_map[point_index][1].append([0, 0, some_sample_num])
        self._set_note_map(note_map)

    def remove_note_map_random_list_entry(self, coords, index):
        note_map = self._get_note_map()
        point_index = self._get_note_map_point_index(coords)

        del note_map[point_index][1][index]
        self._set_note_map(note_map)

    def get_note_map_random_list_sample_id(self, coords, index):
        random_list = self._get_random_list(coords)
        entry = random_list[index]
        return self._get_sample_id(entry[2])

    def set_note_map_random_list_sample_id(self, coords, index, sample_id):
        note_map = self._get_note_map()
        point_index = self._get_note_map_point_index(coords)
        note_map[point_index][1][index][2] = self._get_sample_num(sample_id)
        self._set_note_map(note_map)

    def get_note_map_random_list_cents_offset(self, coords, index):
        random_list = self._get_random_list(coords)
        entry = random_list[index]
        return entry[0]

    def set_note_map_random_list_cents_offset(self, coords, index, offset):
        note_map = self._get_note_map()
        point_index = self._get_note_map_point_index(coords)
        note_map[point_index][1][index][0] = offset
        self._set_note_map(note_map)

    def get_note_map_random_list_volume_adjust(self, coords, index):
        random_list = self._get_random_list(coords)
        entry = random_list[index]
        return entry[1]

    def set_note_map_random_list_volume_adjust(self, coords, index, adjust):
        note_map = self._get_note_map()
        point_index = self._get_note_map_point_index(coords)
        note_map[point_index][1][index][1] = adjust
        self._set_note_map(note_map)


class WavPackValidator():

    def __init__(self, sample_data):
        self._validator = Kunquat()
        self._sample_data = sample_data
        self._validation_error = None

    def is_data_valid(self):
        if self._validation_error:
            return False

        transaction = {
            'au_00/p_manifest.json'                   : {},
            'au_00/proc_00/p_manifest.json'           : { u'type': u'sample' },
            'au_00/proc_00/p_signal_type.json'        : u'voice',
            'au_00/proc_00/c/smp_000/p_sh_sample.json': {
                    u'format': u'WavPack', u'freq'  : 48000, },
            'au_00/proc_00/c/smp_000/p_sample.wv'     : self._sample_data,
        }

        for key, value in transaction.iteritems():
            self._validator.set_data(key, value)

        try:
            self._validator.validate()
        except KunquatFormatError as e:
            self._validation_error = e['message']
            return False

        return True

    def get_validation_error(self):
        return self._validation_error


