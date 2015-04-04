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

from kunquat.kunquat.kunquat import get_default_value


class ChannelDefaults():

    def __init__(self, song_id):
        self._song_id = song_id
        self._controller = None
        self._store = None

    def set_controller(self, controller):
        self._controller = controller
        self._store = controller.get_store()

    def _get_key(self):
        return '{}/p_channel_defaults.json'.format(self._song_id)

    def _get_default_entry(self):
        default_list = get_default_value(self._get_key())
        return default_list[0]

    def _get_entry(self, ch_num):
        key = self._get_key()
        ret_entry = self._get_default_entry()
        stored_list = self._store.get(key, [])
        if ch_num >= len(stored_list):
            return ret_entry
        ret_entry.update(stored_list[ch_num])
        return ret_entry

    def get_default_control_id(self, ch_num):
        entry = self._get_entry(ch_num)
        return 'control_{:02x}'.format(entry['control_id'])

    def set_default_control_id(self, ch_num, control_id):
        key = self._get_key()
        chd_list = self._store.get(key, [])
        def_entry = self._get_default_entry()
        for _ in xrange(ch_num + 1 - len(chd_list)):
            chd_list.append(def_entry.copy())

        control_id_parts = control_id.split('_')
        control_num = int(control_id_parts[1], 16)

        entry = chd_list[ch_num]
        entry['control_id'] = control_num

        self._store[key] = chd_list


