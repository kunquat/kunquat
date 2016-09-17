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

from kunquat.extras.sndfile import SndFileR, SndFileError
from kunquat.extras.wavpack import WavPackRMem, WavPackWMem
from kunquat.kunquat.kunquat import Kunquat, KunquatFormatError
from .procparams import ProcParams


class SampleImportError(ValueError):
    '''Error raised when sample importing fails.'''


class SampleParams(ProcParams):

    _SAMPLES_MAX = 512

    @staticmethod
    def get_default_signal_type():
        return 'voice'

    @staticmethod
    def get_port_info():
        return {
            'in_00':  'pitch',
            'in_01':  'force',
            'out_00': 'audio L',
            'out_01': 'audio R',
        }

    def __init__(self, proc_id, controller):
        super().__init__(proc_id, controller)

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

    def _get_sample_data_handle(self, sample_id):
        data_key = self._get_sample_key(sample_id, 'p_sample.wv')
        data = self._get_value(data_key, None)
        if not data:
            return None
        return WavPackRMem(data)

    def get_max_sample_count(self):
        return self._SAMPLES_MAX

    def get_sample_ids(self):
        ret_ids = []
        for i in range(self._SAMPLES_MAX):
            cur_id = self._get_sample_id(i)
            cur_header = self._get_sample_header(cur_id)
            if type(cur_header) == dict:
                ret_ids.append(cur_id)

        return ret_ids

    def get_free_sample_id(self):
        for i in range(self._SAMPLES_MAX):
            cur_id = self._get_sample_id(i)
            cur_header = self._get_sample_header(cur_id)
            if not type(cur_header) == dict:
                return cur_id
        return None

    def import_sample(self, sample_id, path):
        freq = 48000

        if path.endswith('.wv'):
            with open(path, 'rb') as f:
                sample_data = f.read()

            validator = WavPackValidator(sample_data)
            if not validator.is_data_valid():
                raise SampleImportError('Could not import {}:\n{}'.format(
                    path, validator.get_validation_error()))
            del validator

            wp = WavPackRMem(sample_data)
            freq = wp.get_audio_rate()

        else:
            try:
                sf = SndFileR(path, convert_to_float=False)
                sdata = list(sf.read())
            except SndFileError as e:
                raise SampleImportError('Could not import {}:\n{}'.format(path, str(e)))

            freq = sf.get_audio_rate()

            if sf.get_bits() < 32:
                rshift = 32 - sf.get_bits()
                for buf in sdata:
                    for i in range(len(buf)):
                        buf[i] >>= rshift
            wv = WavPackWMem(
                    sf.get_audio_rate(), sf.get_channels(), sf.is_float(), sf.get_bits())
            wv.write(*sdata)
            sample_data = wv.get_contents()

        sample_data_key = self._get_full_sample_key(sample_id, 'p_sample.wv')
        sample_header_key = self._get_full_sample_key(sample_id, 'p_sh_sample.json')

        header = { 'format': 'WavPack', 'freq': freq, }

        transaction = {}
        transaction[sample_data_key] = sample_data
        transaction[sample_header_key] = header

        self._store.put(transaction)

    def remove_sample(self, sample_id):
        key_prefix = self._get_full_sample_key(sample_id, '')

        transaction = {}
        for key in (k for k in self._store.keys() if k.startswith(key_prefix)):
            transaction[key] = None

        sample_num = self._get_sample_num(sample_id)

        # Remove all references in random lists
        note_map = self._get_note_map()
        for (_, random_list) in note_map:
            random_list[:] = [e for e in random_list if e[0] != sample_num]
        transaction[self._get_conf_key('p_nm_note_map.json')] = note_map

        hit_map = self._get_hit_map()
        for (_, random_list) in hit_map:
            random_list[:] = [e for e in random_list if e[0] != sample_num]
        transaction[self._get_conf_key('p_hm_hit_map.json')] = hit_map

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
        header = self._get_sample_header(sample_id) or {}
        return header.get('freq', 48000)

    def set_sample_freq(self, sample_id, freq):
        header = self._get_sample_header(sample_id)
        header['freq'] = freq
        self._set_sample_header(sample_id, header)

    def get_sample_loop_mode(self, sample_id):
        header = self._get_sample_header(sample_id) or {}
        return header.get('loop_mode', 'off')

    def set_sample_loop_mode(self, sample_id, mode):
        assert mode in ('off', 'uni', 'bi')
        header = self._get_sample_header(sample_id)
        header['loop_mode'] = mode
        self._set_sample_header(sample_id, header)

    def get_sample_loop_start(self, sample_id):
        header = self._get_sample_header(sample_id) or {}
        return header.get('loop_start', 0)

    def set_sample_loop_start(self, sample_id, start):
        header = self._get_sample_header(sample_id)
        header['loop_start'] = start
        self._set_sample_header(sample_id, header)

    def get_sample_loop_end(self, sample_id):
        header = self._get_sample_header(sample_id) or {}
        return header.get('loop_end', 0)

    def set_sample_loop_end(self, sample_id, end):
        header = self._get_sample_header(sample_id)
        header['loop_end'] = end
        self._set_sample_header(sample_id, header)

    def get_sample_length(self, sample_id):
        handle = self._get_sample_data_handle(sample_id)
        if not handle:
            return 0
        return handle.get_length()

    def get_sample_data_retriever(self, sample_id):
        handle = self._get_sample_data_handle(sample_id)
        return lambda: handle.read(65536)

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

    def get_note_map_random_list(self, coords):
        cb_info = {
            'get_map'        : self._get_note_map,
            'get_point_index': self._get_note_map_point_index,
            'set_map'        : self._set_note_map,
            'get_sample_ids' : self.get_sample_ids,
            'get_sample_num' : self._get_sample_num,
            'get_sample_id'  : self._get_sample_id,
        }
        return RandomList(cb_info, coords)

    def _get_hit_map(self):
        return self._get_value('p_hm_hit_map.json', [])

    def _set_hit_map(self, hit_map):
        self._set_value('p_hm_hit_map.json', hit_map)

    def get_selected_hit_info(self):
        return self._session.get_selected_sample_hit_info(self._proc_id)

    def set_selected_hit_info(self, hit_info):
        self._session.set_selected_sample_hit_info(self._proc_id, hit_info)

    def get_selected_hit_map_force(self):
        return self._session.get_selected_sample_hit_map_force(self._proc_id)

    def set_selected_hit_map_force(self, force):
        self._session.set_selected_sample_hit_map_force(self._proc_id, force)

    def _get_hit_index(self, hit_info):
        BANK_SIZE = 32 # TODO: add common hit_info<->hit_index conversion helpers
        hit_base, hit_offset = hit_info
        hit_index = hit_base * BANK_SIZE + hit_offset
        return hit_index

    def get_hit_map_forces(self, hit_info):
        hit_index = self._get_hit_index(hit_info)
        hit_map = self._get_hit_map()
        forces = [key[1] for key, _ in hit_map if key[0] == hit_index]
        return forces

    def add_hit_map_point(self, hit_info, force):
        assert force not in self.get_hit_map_forces(hit_info)
        hit_index = self._get_hit_index(hit_info)
        hit_map = self._get_hit_map()
        hit_map.append([[hit_index, force], []])
        self._set_hit_map(hit_map)

    def _get_hit_map_random_list_key(self, hit_info, force):
        hit_index = self._get_hit_index(hit_info)
        key = [hit_index, force]
        return key

    def _get_hit_map_point_index(self, location):
        hit_info, force = location
        key = self._get_hit_map_random_list_key(hit_info, force)
        hit_map = self._get_hit_map()
        index = [i for i, entry in enumerate(hit_map) if entry[0] == key][0]
        return index

    def move_hit_map_point(self, hit_info, old_force, new_force):
        assert new_force not in self.get_hit_map_forces(hit_info)
        hit_map = self._get_hit_map()
        location = hit_info, old_force
        index = self._get_hit_map_point_index(location)
        hit_map[index][0][1] = new_force
        self._set_hit_map(hit_map)

    def remove_hit_map_point(self, hit_info, force):
        hit_map = self._get_hit_map()
        location = hit_info, force
        index = self._get_hit_map_point_index(location)
        del hit_map[index]
        self._set_hit_map(hit_map)

    def get_hit_map_random_list(self, hit_info, force):
        location = hit_info, force
        cb_info = {
            'get_map'        : self._get_hit_map,
            'get_point_index': self._get_hit_map_point_index,
            'set_map'        : self._set_hit_map,
            'get_sample_ids' : self.get_sample_ids,
            'get_sample_num' : self._get_sample_num,
            'get_sample_id'  : self._get_sample_id,
        }
        return RandomList(cb_info, location)


class RandomList():

    _LENGTH_MAX = 8

    def __init__(self, cb_info, location):
        self._get_map = cb_info['get_map']
        self._get_point_index = cb_info['get_point_index']
        self._set_map = cb_info['set_map']
        self._get_sample_ids = cb_info['get_sample_ids']
        self._get_sample_num = cb_info['get_sample_num']
        self._get_sample_id = cb_info['get_sample_id']
        self._location = location

    def _get_raw_list(self):
        raw_map = self._get_map()
        point_index = self._get_point_index(self._location)
        return raw_map[point_index][1]

    def get_length(self):
        raw_list = self._get_raw_list()
        return len(raw_list)

    def is_full(self):
        return (self.get_length() >= self._LENGTH_MAX)

    def add_entry(self):
        raw_map = self._get_map()
        point_index = self._get_point_index(self._location)

        sample_ids = self._get_sample_ids()
        some_sample_num = self._get_sample_num(sample_ids[0]) if sample_ids else 0

        raw_map[point_index][1].append([some_sample_num, 0, 0])
        self._set_map(raw_map)

    def remove_entry(self, index):
        raw_map = self._get_map()
        point_index = self._get_point_index(self._location)

        del raw_map[point_index][1][index]
        self._set_map(raw_map)

    def get_sample_id(self, index):
        raw_list = self._get_raw_list()
        entry = raw_list[index]
        return self._get_sample_id(entry[0])

    def set_sample_id(self, index, sample_id):
        raw_map = self._get_map()
        point_index = self._get_point_index(self._location)
        raw_map[point_index][1][index][0] = self._get_sample_num(sample_id)
        self._set_map(raw_map)

    def get_cents_offset(self, index):
        raw_list = self._get_raw_list()
        entry = raw_list[index]
        return entry[1]

    def set_cents_offset(self, index, offset):
        raw_map = self._get_map()
        point_index = self._get_point_index(self._location)
        raw_map[point_index][1][index][1] = offset
        self._set_map(raw_map)

    def get_volume_adjust(self, index):
        raw_list = self._get_raw_list()
        entry = raw_list[index]
        return entry[2]

    def set_volume_adjust(self, index, adjust):
        raw_map = self._get_map()
        point_index = self._get_point_index(self._location)
        raw_map[point_index][1][index][2] = adjust
        self._set_map(raw_map)


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
            'au_00/proc_00/p_manifest.json'           : { 'type': 'sample' },
            'au_00/proc_00/p_signal_type.json'        : 'voice',
            'au_00/proc_00/c/smp_000/p_sh_sample.json': {
                    'format': 'WavPack', 'freq' : 48000, },
            'au_00/proc_00/c/smp_000/p_sample.wv'     : self._sample_data,
        }

        for key, value in transaction.items():
            self._validator.set_data(key, value)

        try:
            self._validator.validate()
        except KunquatFormatError as e:
            self._validation_error = e['message']
            return False

        return True

    def get_validation_error(self):
        return self._validation_error


