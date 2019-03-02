# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from itertools import chain
import re

from kunquat.kunquat.kunquat import get_default_value
from kunquat.kunquat.limits import *
from kunquat.tracker.ui.controller.kqtifile import KqtiFile
from kunquat.tracker.version import KUNQUAT_VERSION
from .audiounit import AudioUnit
from .bindings import Bindings
from .channeldefaults import ChannelDefaults
from .connections import Connections
from .control import Control
from .album import Album
from .environment import Environment
from .tuningtable import TuningTable


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

    def get_editor_version(self):
        return self._store.get('m_editor_version.json')

    def set_editor_version(self):
        self._store.put(
                { 'm_editor_version.json': KUNQUAT_VERSION }, mark_modified=False)
        for au in self.get_audio_units():
            au.set_editor_version()

    def get_title(self):
        return self._store.get('m_title.json')

    def set_title(self, title):
        self._store['m_title.json'] = title

    def get_name(self):
        return self.get_title()

    def _get_authors(self):
        return self._store.get('m_authors.json', [])

    def get_author_count(self):
        return len(self._get_authors())

    def get_author(self, index):
        return self._get_authors()[index]

    def set_author(self, index, name):
        authors = self._get_authors()
        if name:
            if index == len(authors):
                authors.append(name)
            else:
                authors[index] = name
        else:
            if index < len(authors):
                del authors[index]
        self._store['m_authors.json'] = authors

    def get_message(self):
        return self._store.get('m_message.json', '')

    def set_message(self, message):
        self._store['m_message.json'] = message or None

    def get_dc_blocker_enabled(self):
        key = 'p_dc_blocker_enabled.json'
        return self._store.get(key, get_default_value(key))

    def set_dc_blocker_enabled(self, enabled):
        self._store['p_dc_blocker_enabled.json'] = enabled

    def get_mixing_volume(self):
        key = 'p_mixing_volume.json'
        return self._store.get(key, get_default_value(key))

    def set_mixing_volume(self, volume):
        self._store['p_mixing_volume.json'] = volume

    def get_force_shift(self):
        key = 'p_force_shift.json'
        return self._store.get(key, get_default_value(key))

    def set_force_shift(self, value):
        self._store['p_force_shift.json'] = value

    def get_random_seed(self):
        key = 'p_random_seed.json'
        return self._store.get(key, get_default_value(key))

    def set_random_seed(self, value):
        self._store['p_random_seed.json'] = value

    def get_random_seed_auto_update(self):
        return self._store.get('i_random_seed_auto_update.json', False)

    def set_random_seed_auto_update(self, enabled):
        self._store['i_random_seed_auto_update.json'] = enabled

    def get_control_ids(self):
        key = 'p_control_map.json'
        input_map = self._store.get(key, get_default_value(key))
        control_ids = set()
        for (control_number, _) in input_map:
            control_id = 'control_{0:02x}'.format(control_number)
            control_ids.add(control_id)
        return control_ids

    def get_free_control_id(self):
        all_control_ids = set('control_{:02x}'.format(i) for i in range(CONTROLS_MAX))
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

    def get_load_error_info(self):
        return self._session.get_module_load_error_info()

    def get_clear_save_error_info(self):
        return self._session.get_clear_module_save_error_info()

    def get_reset_au_import_error_info(self):
        return self._session.get_reset_au_import_error_info()

    def get_reset_au_export_error_info(self):
        return self._session.get_reset_au_export_error_info()

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
        all_au_ids = set('au_{:02x}'.format(i) for i in range(AUDIO_UNITS_MAX))
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

    def _get_edit_try_connect_au_to_master(self, au_id):
        aus = self.get_audio_units()
        has_effects = any(au.is_effect() for au in aus)
        if has_effects:
            return {}

        conns = self.get_connections()
        transaction = conns.get_edit_connect_ports(au_id, 'out_00', 'master', 'out_00')
        transaction.update(conns.get_edit_connect_ports(
            au_id, 'out_01',
            'master', 'out_01',
            transaction))
        return transaction

    def add_instrument_with_control(self, au_id, control_id, ins_name='New instrument'):
        au = AudioUnit(au_id)
        au.set_controller(self._controller)
        au.set_ui_model(self._ui_model)

        control = Control(control_id)
        control.set_controller(self._controller)
        control.set_ui_model(self._ui_model)

        transaction = au.get_edit_create_new_instrument(ins_name)
        transaction.update(control.get_edit_create_new())
        transaction.update(control.get_edit_connect_to_au(au_id))
        transaction.update(self._get_edit_try_connect_au_to_master(au_id))

        self._store.put(transaction)

    def add_effect(self, au_id, control_id):
        au = AudioUnit(au_id)
        au.set_controller(self._controller)
        au.set_ui_model(self._ui_model)

        control = Control(control_id)
        control.set_controller(self._controller)
        control.set_ui_model(self._ui_model)

        transaction = au.get_edit_create_new_effect()
        transaction.update(control.get_edit_create_new())
        transaction.update(control.get_edit_connect_to_au(au_id))

        self._store.put(transaction)

    def get_out_ports(self):
        out_ports = []
        for i in range(0x100):
            port_id = 'out_{:02x}'.format(i)
            key = '{}/p_manifest.json'.format(port_id)
            if key in self._store:
                out_ports.append(port_id)

        return out_ports

    def get_port_info(self):
        return { 'out_00': 'out L', 'out_01': 'out R' }

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

    def get_bindings(self):
        bindings = Bindings()
        bindings.set_controller(self._controller)
        return bindings

    def remove_controls_to_audio_unit(self, au_id):
        transaction = {}

        # Update channel defaults
        fallback_au_ids = self.get_au_ids()
        fallback_au_ids.discard(au_id)
        fallback_au_id = min(fallback_au_ids) if fallback_au_ids else 'au_00'
        if fallback_au_id != au_id:
            fallback_control_id = (
                    self.get_control_id_by_au_id(fallback_au_id) or 'control_00')
            ch_defs = self.get_channel_defaults()
            transaction.update(ch_defs.get_edit_remove_controls_to_audio_unit(
                au_id, fallback_control_id))

        # Remove controls
        remove_control_nums = []
        for i in range(CONTROLS_MAX):
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
        for key in self._store.keys():
            if key.startswith(start):
                transaction[key] = None

        self._store.put(transaction)

    def get_tuning_table_ids(self):
        table_ids = set()
        pat = re.compile('tuning_[0-9a-f]{2}/p_tuning_table.json')
        for key in self._store.keys():
            if pat.match(key):
                table_id = key.split('/')[0]
                num = int(table_id.split('_')[1], 16)
                if num < TUNING_TABLES_MAX:
                    table_ids.add(table_id)
        return table_ids

    def get_free_tuning_table_id(self):
        all_ids = set('tuning_{:02x}'.format(i) for i in range(TUNING_TABLES_MAX))
        used_ids = self.get_tuning_table_ids()
        free_ids = all_ids - used_ids
        if not free_ids:
            return None
        return min(free_ids)

    def add_tuning_table(self, table_id):
        assert table_id not in self.get_tuning_table_ids()
        table_key = '{}/p_tuning_table.json'.format(table_id)
        name_key = '{}/m_name.json'.format(table_id)
        note_names_key = '{}/m_note_names.json'.format(table_id)
        transaction = {
            table_key     : get_default_value(table_key),
            name_key      : 'Tuning {}'.format(int(table_id.split('_')[1], 16)),
            note_names_key: ['A'],
        }
        self._store.put(transaction)

    def create_tuning_table_from_notation_template(
            self, table_id, name, notation_template):
        assert table_id not in self.get_tuning_table_ids()
        tt = TuningTable(table_id)
        tt.set_controller(self._controller)
        tt.apply_notation_template(name, notation_template)

    def get_tuning_table(self, table_id):
        tt = TuningTable(table_id)
        tt.set_controller(self._controller)
        return tt

    def set_path(self, path):
        self._session.set_module_path(path)

    def get_path(self):
        return self._session.get_module_path()

    def execute_load(self, task_executor):
        assert not self.is_saving()
        task = self._controller.get_task_load_module(self.get_path())
        task_executor(task)

    def finalise_create_sandbox(self):
        yield # make this a generator
        self._store['i_random_seed_auto_update.json'] = True
        self._store.clear_modified_flag()
        self._updater.signal_update('signal_module')

    def execute_create_sandbox(self, task_executor):
        assert not self.is_saving()
        self._controller.create_sandbox()
        self.add_instrument_with_control('au_00', 'control_00', 'Sine')
        finalise_task = self.finalise_create_sandbox()
        task_executor(finalise_task)

    def start_import_au(self, path, au_id, control_id=None):
        assert not self.is_saving()
        assert not self.is_importing_audio_unit()
        self._session.set_au_import_info((path, au_id, control_id))
        self._updater.signal_update('signal_start_import_au')

    def execute_import_au(self, task_executor):
        assert self.is_importing_audio_unit()
        path, au_id, control_id = self._session.get_au_import_info()
        kqtifile = KqtiFile(path)
        load_task = self._controller.get_task_load_audio_unit(
                kqtifile, au_id, control_id)
        task_executor(load_task)

    def finish_import_au(self):
        self._session.set_au_import_info(None)

    def is_importing_audio_unit(self):
        return self._session.is_importing_audio_unit()

    def execute_export_au(self, task_executor):
        au_id, path = self._session.get_au_export_info()
        self._session.set_au_export_info(None)
        task = self._controller.get_task_export_audio_unit(au_id, path)
        task_executor(task)

    def finish_export_au(self):
        self._store.set_saving(False)
        self._session.set_saving(False)

    def is_saving(self):
        return self._session.is_saving()

    def start_save(self):
        assert not self.is_saving()
        assert not self.is_importing_audio_unit()

        self.set_editor_version()

        self._session.set_saving(True)
        self._updater.signal_update('signal_start_save_module')

    def flush(self, callback):
        self._store.flush(callback)

    def execute_save(self, task_executor):
        assert self.is_saving()

        self._store.set_saving(True)

        module_path = self._session.get_module_path()
        task = self._controller.get_task_save_module(module_path)
        task_executor(task)

    def finish_save(self):
        self._store.clear_modified_flag()
        self._store.set_saving(False)
        self._session.set_saving(False)

    def finish_save_with_error(self):
        self.set_path(None)
        self._store.set_saving(False)
        self._session.set_saving(False)


