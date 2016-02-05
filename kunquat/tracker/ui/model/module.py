# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from itertools import chain

from kunquat.kunquat.kunquat import get_default_value
from kunquat.kunquat.limits import *
from audiounit import AudioUnit
from channeldefaults import ChannelDefaults
from connections import Connections
from control import Control
from album import Album
from environment import Environment


class Module():

    def __init__(self):
        self._updater = None
        self._session = None
        self._store = None
        self._controller = None
        self._ui_model = None
        self._audio_units = {}

    def set_controller(self, controller):
        self._updater = controller.get_updater()
        self._session = controller.get_session()
        self._store = controller.get_store()
        self._controller = controller

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def is_modified(self):
        return self._store.is_modified()

    def get_title(self):
        return self._store.get('m_title.json')

    def get_name(self):
        return self.get_title()

    def get_control_ids(self):
        key = 'p_control_map.json'
        input_map = self._store.get(key, get_default_value(key))
        control_ids = set()
        for (control_number, _) in input_map:
            control_id = 'control_{0:02x}'.format(control_number)
            control_ids.add(control_id)
        return control_ids

    def get_free_control_id(self):
        all_control_ids = set('control_{:02x}'.format(i) for i in xrange(CONTROLS_MAX))
        used_control_ids = self.get_control_ids()
        free_control_ids = all_control_ids - used_control_ids
        if not free_control_ids:
            return None
        return min(free_control_ids)

    def get_control_id_by_au_id(self, au_id):
        if '/' in au_id:
            return None
        search_au_num = int(au_id.split('_')[1], 16)
        control_map = self._store.get('p_control_map.json', [])
        for control_num, au_num in control_map:
            if au_num == search_au_num:
                control_id = 'control_{0:02x}'.format(control_num)
                return control_id
        return None

    def get_control(self, control_id):
        control = Control(control_id)
        control.set_controller(self._controller)
        control.set_ui_model(self._ui_model)
        return control

    def add_control(self, control_id):
        control = Control(control_id)
        control.set_controller(self._controller)
        control.set_ui_model(self._ui_model)
        control.set_existence(True)

    def get_audio_unit(self, au_id):
        au = AudioUnit(au_id)
        au.set_controller(self._controller)
        au.set_ui_model(self._ui_model)
        return au

    def get_au_ids(self):
        au_ids = set()
        for key in self._store.keys():
            if key.startswith('au_'):
                au_id = key.split('/')[0]
                au_ids.add(au_id)
        return au_ids

    def get_free_au_id(self):
        all_au_ids = set('au_{:02x}'.format(i) for i in xrange(AUDIO_UNITS_MAX))
        used_au_ids = self.get_au_ids()
        free_au_ids = all_au_ids - used_au_ids
        if not free_au_ids:
            return None
        return min(free_au_ids)

    def get_audio_units(self, validate=True):
        au_ids = self.get_au_ids()
        all_audio_units = [self.get_audio_unit(i) for i in au_ids]
        #all_auidio_units = self._audio_units.values()
        #if validate:
        #    valid = [i for i in all_audio_units if i.get_existence()]
        #    return [] #valid
        return all_audio_units

    def add_instrument(self, au_id):
        au = AudioUnit(au_id)
        au.set_controller(self._controller)
        au.set_ui_model(self._ui_model)
        au.set_existence('instrument')
        au.set_port_existence('in_00', True)
        au.set_port_existence('in_01', True)
        au.set_port_existence('out_00', True)
        au.set_port_existence('out_01', True)

    def add_effect(self, au_id):
        au = AudioUnit(au_id)
        au.set_controller(self._controller)
        au.set_ui_model(self._ui_model)
        au.set_existence('effect')
        au.set_port_existence('in_00', True)
        au.set_port_existence('in_01', True)
        au.set_port_existence('out_00', True)
        au.set_port_existence('out_01', True)

    def get_out_ports(self):
        out_ports = []
        for i in xrange(0x100):
            port_id = 'out_{:02x}'.format(i)
            key = '{}/p_manifest.json'.format(port_id)
            if key in self._store:
                out_ports.append(port_id)

        return out_ports

    def get_port_info(self):
        return { 'out_00': u'out L', 'out_01': u'out R' }

    def get_connections(self):
        connections = Connections()
        connections.set_controller(self._controller)
        connections.set_ui_model(self._ui_model)
        return connections

    def get_album(self):
        album = Album()
        album.set_controller(self._controller)
        album.set_ui_model(self._ui_model)
        return album

    def get_channel_defaults(self):
        ch_defs = ChannelDefaults()
        ch_defs.set_controller(self._controller)
        ch_defs.set_ui_model(self._ui_model)
        return ch_defs

    def get_environment(self):
        env = Environment()
        env.set_controller(self._controller)
        return env

    def remove_controls_to_audio_unit(self, au_id):
        transaction = {}

        # Update channel defaults
        fallback_au_ids = self.get_au_ids()
        fallback_au_ids.discard(au_id)
        fallback_au_id = min(fallback_au_ids) if fallback_au_ids else 'au_00'
        if fallback_au_id == au_id:
            return
        fallback_control_id = (
                self.get_control_id_by_au_id(fallback_au_id) or 'control_00')

        ch_defs = self.get_channel_defaults()
        transaction.update(ch_defs.get_edit_remove_controls_to_audio_unit(
            au_id, fallback_control_id))

        # Remove controls
        remove_control_nums = []
        for i in xrange(CONTROLS_MAX):
            control_id = 'control_{:02x}'.format(i)
            control = Control(control_id)
            control.set_controller(self._controller)
            control.set_ui_model(self._ui_model)
            if control.get_existence() and (control.get_audio_unit().get_id() == au_id):
                remove_control_nums.append(i)
                transaction.update(control.get_edit_remove_control())

        # Update the control map
        cmap_key = 'p_control_map.json'
        control_map = self._store.get(cmap_key, get_default_value(cmap_key))
        new_map = []
        for pair in control_map:
            cnum, au_num = pair
            cur_au_id = 'au_{:02x}'.format(au_num)
            if cur_au_id != au_id:
                new_map.append(pair)
        transaction.update({ cmap_key: new_map })

        self._store.put(transaction)

    def remove_audio_unit(self, au_id):
        transaction = {}
        start = au_id + '/'
        for key in self._store.iterkeys():
            if key.startswith(start):
                transaction[key] = None

        self._store.put(transaction)

    def set_path(self, path):
        self._session.set_module_path(path)

    def get_path(self):
        return self._session.get_module_path()

    def execute_load(self, task_executer):
        assert not self.is_saving()
        task = self._controller.get_task_load_module(self.get_path())
        task_executer(task)

    def finalise_create_sandbox(self):
        yield # make this a generator
        self._store.clear_modified_flag()

    def execute_create_sandbox(self, task_executer):
        assert not self.is_saving()
        self._controller.create_sandbox()
        kqtifile = self._controller.get_share().get_default_instrument()
        load_au_task = self._controller.get_task_load_audio_unit(kqtifile)
        finalise_task = self.finalise_create_sandbox()
        task = chain(load_au_task, finalise_task)
        task_executer(task)

    def is_saving(self):
        return self._session.is_saving()

    def start_save(self):
        assert not self.is_saving()
        self._session.set_saving(True)
        self._store.set_saving(True)
        self._store.clear_modified_flag()
        self._updater.signal_update(set(['signal_start_save_module']))

    def flush(self, callback):
        self._store.flush(callback)

    def execute_save(self, task_executer):
        assert self.is_saving()
        module_path = self._session.get_module_path()
        task = self._controller.get_task_save_module(module_path)
        task_executer(task)

    def finish_save(self):
        self._store.set_saving(False)
        self._session.set_saving(False)


