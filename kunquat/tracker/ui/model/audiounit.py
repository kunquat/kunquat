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

from kunquat.kunquat.kunquat import get_default_value
from kunquat.kunquat.limits import *
from kunquat.tracker.version import KUNQUAT_VERSION
from .connections import Connections
from .hit import Hit
from .processor import Processor
from . import tstamp


class AudioUnit():

    def __init__(self, au_id):
        assert au_id
        assert len(au_id.split('/')) <= 2
        self._au_id = au_id
        self._store = None
        self._controller = None
        self._session = None
        self._ui_model = None
        self._updater = None

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller
        self._session = controller.get_session()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

    def get_id(self):
        return self._au_id

    def get_edit_create_new_instrument(self):
        transaction = self.get_edit_set_existence('instrument')
        transaction.update(self.get_edit_set_port_existence('out_00', True))
        transaction.update(self.get_edit_set_port_existence('out_01', True))
        transaction.update(self.get_edit_set_port_name('out_00', 'audio L'))
        transaction.update(self.get_edit_set_port_name('out_01', 'audio R'))
        return transaction

    def _get_key(self, subkey):
        return '{}/{}'.format(self._au_id, subkey)

    def get_edit_set_existence(self, au_type):
        assert (not au_type) or (au_type in ('instrument', 'effect'))
        key = self._get_key('p_manifest.json')
        if au_type:
            manifest = { 'type': au_type }
        else:
            manifest = None
        return { key: manifest }

    def set_existence(self, au_type):
        transaction = self.get_edit_set_existence(au_type)
        self._store.put(transaction)

    def get_existence(self):
        key = self._get_key('p_manifest.json')
        manifest = self._store.get(key, None)
        return (type(manifest) == dict)

    def set_editor_version(self):
        key = self._get_key('m_editor_version.json')
        self._store.put({ key: KUNQUAT_VERSION }, mark_modified=False)

        for au_id in self.get_au_ids():
            au = self.get_audio_unit(au_id)
            au.set_editor_version()

    def get_editor_version(self):
        key = self._get_key('m_editor_version.json')
        self._store.get(key)

    def is_instrument(self):
        key = self._get_key('p_manifest.json')
        manifest = self._store[key]
        return self.get_existence() and (manifest.get('type') == 'instrument')

    def is_effect(self):
        key = self._get_key('p_manifest.json')
        manifest = self._store[key]
        return self.get_existence() and (manifest.get('type') == 'effect')

    def get_in_ports(self):
        in_ports = []
        for i in range(0x100):
            port_id = 'in_{:02x}'.format(i)
            key = self._get_key('{}/p_manifest.json'.format(port_id))
            if key in self._store:
                in_ports.append(port_id)

        return in_ports

    def get_out_ports(self):
        out_ports = []
        for i in range(0x100):
            port_id = 'out_{:02x}'.format(i)
            key = self._get_key('{}/p_manifest.json'.format(port_id))
            if key in self._store:
                out_ports.append(port_id)

        return out_ports

    def get_port_info(self):
        info = {}
        for in_port_id in self.get_in_ports():
            info[in_port_id] = self.get_port_name(in_port_id)
        for out_port_id in self.get_out_ports():
            info[out_port_id] = self.get_port_name(out_port_id)
        return info

    def get_edit_set_port_existence(self, port_id, existence):
        key = self._get_key('{}/p_manifest.json'.format(port_id))
        manifest = {} if existence else None
        return { key: manifest }

    def set_port_existence(self, port_id, existence):
        transaction = self.get_edit_set_port_existence(port_id, existence)
        self._store.put(transaction)

    def get_free_input_port_id(self):
        for i in range(0x100):
            port_id = 'in_{:02x}'.format(i)
            key = self._get_key('{}/p_manifest.json'.format(port_id))
            if key not in self._store:
                return port_id
        return None

    def get_free_output_port_id(self):
        for i in range(0x100):
            port_id = 'out_{:02x}'.format(i)
            key = self._get_key('{}/p_manifest.json'.format(port_id))
            if key not in self._store:
                return port_id
        return None

    def get_port_name(self, port_id):
        key = self._get_key('{}/m_name.json'.format(port_id))
        return self._store.get(key, None)

    def get_edit_set_port_name(self, port_id, name):
        key = self._get_key('{}/m_name.json'.format(port_id))
        return { key: name }

    def set_port_name(self, port_id, name):
        transaction = self.get_edit_set_port_name(port_id, name)
        self._store.put(transaction)

    def remove_port(self, port_id):
        transaction = {
            self._get_key('{}/p_manifest.json'.format(port_id)): None,
            self._get_key('{}/m_name.json'.format(port_id)):     None,
        }

        # Remove internal connections to the removed port
        conns = self.get_connections()
        transaction.update(conns.get_edit_disconnect_master_port(port_id))

        # Remove external connections to the removed port
        module = self._ui_model.get_module()
        if '/' in self._au_id:
            parent_au_id = self._au_id.split('/')[0]
            parent_conns = module.get_audio_unit(parent_au_id).get_connections()
        else:
            parent_conns = module.get_connections()
        transaction.update(parent_conns.get_edit_disconnect_port(self._au_id, port_id))

        self._store.put(transaction)

    def get_connections(self):
        connections = Connections()
        connections.set_au_id(self._au_id)
        connections.set_controller(self._controller)
        connections.set_ui_model(self._ui_model)
        return connections

    def set_connections_edit_mode(self, mode):
        assert mode in ('normal', 'hit_proc_filter', 'expr_filter')
        self._session.set_au_connections_edit_mode(self._au_id, mode)

    def get_connections_edit_mode(self):
        return self._session.get_au_connections_edit_mode(self._au_id) or 'normal'

    def get_processor(self, proc_id):
        proc = Processor(self._au_id, proc_id)
        proc.set_controller(self._controller)
        return proc

    def get_processor_ids(self):
        proc_ids = set()
        for key in self._store.keys():
            start = '{}/proc_'.format(self._au_id)
            if key.startswith(start):
                proc_id_pos = len(self._au_id.split('/'))
                key_parts = key.split('/')
                proc_id = '/'.join(key_parts[:proc_id_pos + 1])
                proc_ids.add(proc_id)
        return proc_ids

    def get_free_processor_id(self):
        used_proc_ids = self.get_processor_ids()
        all_proc_ids = set('{}/proc_{:02x}'.format(self._au_id, i)
                for i in range(PROCESSORS_MAX))
        free_proc_ids = all_proc_ids - used_proc_ids
        free_list = sorted(list(free_proc_ids))
        if not free_list:
            return None
        return free_list[0]

    def add_processor(self, proc_id, proc_type):
        assert proc_id.startswith(self._au_id)
        key_prefix = proc_id
        transaction = {}

        manifest_key = '{}/p_manifest.json'.format(key_prefix)
        transaction[manifest_key] = { 'type': proc_type }

        params_class = Processor.get_params_class(proc_type)

        signal_type_key = '{}/p_signal_type.json'.format(key_prefix)
        transaction[signal_type_key] = params_class.get_default_signal_type()

        for port_id in params_class.get_port_info().keys():
            port_manifest_key = '{}/{}/p_manifest.json'.format(key_prefix, port_id)
            transaction[port_manifest_key] = {}

        self._store.put(transaction)

    def _get_hit_key_base(self, hit_index):
        return self._get_key('hit_{:02x}'.format(hit_index))

    def has_hits(self):
        return any(self.get_hit(i).get_existence() for i in range(HITS_MAX))

    def get_hit(self, hit_index):
        hit = Hit(self._au_id, hit_index)
        hit.set_controller(self._controller)
        return hit

    def set_connections_hit_index(self, hit_index):
        self._session.set_au_connections_hit_index(self._au_id, hit_index)

    def get_connections_hit_index(self):
        return self._session.get_au_connections_hit_index(self._au_id)

    def set_edit_selected_hit_info(self, hit_base, hit_offset):
        self._session.set_edit_selected_hit_info(self._au_id, hit_base, hit_offset)

    def get_edit_selected_hit_info(self):
        return self._session.get_edit_selected_hit_info(self._au_id)

    def _set_expressions(self, expressions):
        key = self._get_key('p_expressions.json')
        self._store[key] = expressions

    def _get_expressions(self):
        key = self._get_key('p_expressions.json')
        stored_expressions = self._store.get(key, {})
        expressions = { 'expressions': {}, 'default_note_expr': '' }
        expressions.update(stored_expressions)
        return expressions

    def has_expressions(self):
        return bool(self._get_expressions()['expressions'])

    def has_expression(self, name):
        return name in self._get_expressions()['expressions']

    def get_expression_names(self):
        expressions = self._get_expressions()
        return list(expressions['expressions'].keys())

    def set_selected_expression(self, name):
        self._session.set_selected_expression(self._au_id, name)

    def get_selected_expression(self):
        return self._session.get_selected_expression(self._au_id)

    def add_expression(self):
        expressions_def = self._get_expressions()
        expressions = expressions_def['expressions']

        init_names = ('expr{:02d}'.format(i) for i in range(len(expressions) + 1))
        for name in init_names:
            if name not in expressions:
                unique_name = name
                break
        expressions[unique_name] = []

        expressions_def['expressions'] = expressions
        self._set_expressions(expressions_def)

    def remove_expression(self, name):
        expressions_def = self._get_expressions()
        expressions = expressions_def['expressions']
        del expressions[name]
        expressions_def['expressions'] = expressions
        if (expressions_def['default_note_expr'] and
                expressions_def['default_note_expr'] not in expressions):
            expressions_def['default_note_expr'] = ''
        self._set_expressions(expressions_def)

    def change_expression_name(self, old_name, new_name):
        expressions_def = self._get_expressions()
        expressions = expressions_def['expressions']
        expr_info = expressions.pop(old_name)
        expressions[new_name] = expr_info
        expressions_def['expressions'] = expressions
        if expressions_def['default_note_expr'] == old_name:
            expressions_def['default_note_expr'] = new_name
        self._set_expressions(expressions_def)

    def set_default_note_expression(self, name):
        expressions = self._get_expressions()
        expressions['default_note_expr'] = name
        self._set_expressions(expressions)

    def get_default_note_expression(self):
        expressions = self._get_expressions()
        return expressions['default_note_expr']

    def set_expression_proc_filter(self, name, proc_filter):
        expressions_def = self._get_expressions()
        expressions = expressions_def['expressions']
        expressions[name] = proc_filter
        expressions_def['expressions'] = expressions
        self._set_expressions(expressions_def)

    def get_expression_proc_filter(self, name):
        expressions = self._get_expressions()
        return expressions['expressions'][name]

    def set_connections_expr_name(self, expr_name):
        self._session.set_au_connections_expr_name(self._au_id, expr_name)

    def get_connections_expr_name(self):
        return self._session.get_au_connections_expr_name(self._au_id)

    def set_test_force(self, force):
        self._session.set_au_test_force(self._au_id, force)

    def get_test_force(self):
        return self._session.get_au_test_force(self._au_id)

    def set_test_expression(self, index, expr_name):
        self._session.set_au_test_expression(self._au_id, index, expr_name)

    def get_test_expression(self, index):
        return self._session.get_au_test_expression(self._au_id, index)

    def set_test_params_enabled(self, enabled):
        self._session.set_au_test_params_enabled(self._au_id, enabled)

    def get_audio_unit(self, au_id):
        assert au_id.startswith(self._au_id + '/')
        assert len(au_id.split('/')) == len(self._au_id.split('/')) + 1
        au = AudioUnit(au_id)
        au.set_controller(self._controller)
        au.set_ui_model(self._ui_model)
        return au

    def get_au_ids(self):
        au_ids = set()
        start = '{}/au_'.format(self._au_id)
        for key in self._store.keys():
            if key.startswith(start):
                sub_au_id_pos = len(self._au_id.split('/'))
                key_parts = key.split('/')
                au_id = '/'.join(key_parts[:sub_au_id_pos + 1])
                au_ids.add(au_id)
        return au_ids

    def get_free_au_id(self):
        all_au_ids = set('{}/au_{:02x}'.format(self._au_id, i)
                for i in range(AUDIO_UNITS_MAX))
        used_au_ids = self.get_au_ids()
        free_au_ids = all_au_ids - used_au_ids
        if not free_au_ids:
            return None
        return min(free_au_ids)

    def start_import_au(self, path, au_id, control_id=None):
        assert control_id == None
        module = self._ui_model.get_module()
        module.start_import_au(path, au_id)

    def start_export_au(self, path):
        module = self._ui_model.get_module()
        assert not module.is_saving()
        assert not module.is_importing_audio_unit()

        self.set_editor_version()

        self._session.set_saving(True)
        self._store.set_saving(True)
        self._session.set_au_export_info((self._au_id, path))
        self._updater.signal_update('signal_start_export_au')

    def add_effect(self, au_id):
        key = '/'.join((self._au_id, au_id))
        au = AudioUnit(au_id)
        au.set_controller(self._controller)
        au.set_ui_model(self._ui_model)
        au.set_existence('effect')
        au.set_port_existence('in_00', True)
        au.set_port_existence('in_01', True)
        au.set_port_existence('out_00', True)
        au.set_port_existence('out_01', True)
        au.set_port_name('in_00', 'audio L')
        au.set_port_name('in_01', 'audio R')
        au.set_port_name('out_00', 'audio L')
        au.set_port_name('out_01', 'audio R')

    def _remove_device(self, dev_id):
        assert dev_id.startswith(self._au_id)

        transaction = {}
        start = dev_id + '/'
        for key in self._store.keys():
            if key.startswith(start):
                transaction[key] = None

        id_parts = dev_id.split('/')
        if id_parts[1].startswith('proc_'):
            # Remove event stream interfaces pointing at the device
            removed_proc_num = int(id_parts[1].split('_')[1], 16)
            stream_list = self._get_stream_list()
            new_stream_list = [
                    [name, num] for name, num in stream_list if num != removed_proc_num]
            transaction[self._get_key('p_streams.json')] = new_stream_list

        self._store.put(transaction)

    def remove_audio_unit(self, au_id):
        self._remove_device(au_id)

    def remove_processor(self, proc_id):
        self._remove_device(proc_id)

    def get_name(self):
        key = self._get_key('m_name.json')
        try:
            name = self._store[key]
        except KeyError:
            return None
        return name

    def set_name(self, name):
        assert isinstance(name, str)
        key = self._get_key('m_name.json')
        self._store[key] = name

    def get_message(self):
        key = self._get_key('m_message.json')
        return self._store.get(key, '')

    def set_message(self, message):
        key = self._get_key('m_message.json')
        self._store[key] = message or None

    def _get_stream_list(self):
        key = self._get_key('p_streams.json')
        ret = self._store.get(key, get_default_value(key))
        return ret

    def _set_stream_list(self, stream_list):
        key = self._get_key('p_streams.json')
        self._store[key] = stream_list

    def get_stream_names(self):
        stream_list = self._get_stream_list()
        return [name for (name, _) in stream_list]

    def _get_stream_entry_index(self, stream_name):
        stream_list = self._get_stream_list()
        for i, entry in enumerate(stream_list):
            if entry[0] == stream_name:
                return i
        raise ValueError('Stream {} not in list'.format(stream_name))

    def get_stream_target_processor(self, stream_name):
        stream_list = self._get_stream_list()
        index = self._get_stream_entry_index(stream_name)
        entry = stream_list[index]
        return entry[1]

    def _get_unique_stream_name(self, stream_list):
        names = set('var{:02d}'.format(i) for i in range(1, len(stream_list) + 2))
        for entry in stream_list:
            used_name = entry[0]
            names.discard(used_name)
        unique_name = min(names)
        return unique_name

    def get_stream_processor_ids(self):
        proc_ids = self.get_processor_ids()
        stream_proc_ids = set(pid for pid in proc_ids
                if self.get_processor(pid).get_type() == 'stream')
        return stream_proc_ids

    def add_stream(self):
        stream_proc_ids = self.get_stream_processor_ids()
        assert len(stream_proc_ids) > 0

        stream_list = self._get_stream_list()
        stream_name = self._get_unique_stream_name(stream_list)

        min_proc_id = min(stream_proc_ids)
        min_proc_num = int(min_proc_id.split('_')[-1], 16)
        new_entry = [stream_name, min_proc_num]

        stream_list.append(new_entry)
        self._set_stream_list(stream_list)

    def change_stream_name(self, old_stream_name, new_stream_name):
        stream_list = self._get_stream_list()
        index = self._get_stream_entry_index(old_stream_name)
        stream_list[index][0] = new_stream_name
        self._set_stream_list(stream_list)

    def set_stream_target_processor(self, stream_name, proc_num):
        stream_list = self._get_stream_list()
        index = self._get_stream_entry_index(stream_name)
        stream_list[index][1] = proc_num
        self._set_stream_list(stream_list)

    def remove_stream(self, stream_name):
        stream_list = self._get_stream_list()
        index = self._get_stream_entry_index(stream_name)
        del stream_list[index]
        self._set_stream_list(stream_list)

    def _get_event_list(self):
        key = self._get_key('p_events.json')
        ret = self._store.get(key, get_default_value(key))
        return ret

    def _get_event_dict(self):
        event_list = self._get_event_list()
        event_dict = {}
        for entry in event_list:
            event_name, arg_type_name, bindings = entry
            event_dict[event_name] = (arg_type_name, bindings)
        return event_dict

    def _get_event_entry_index(self, event_name):
        event_list = self._get_event_list()
        for i, entry in enumerate(event_list):
            if entry[0] == event_name:
                return i
        raise ValueError('Event name {} not in list'.format(event_name))

    def _set_event_list(self, event_list):
        key = self._get_key('p_events.json')
        self._store[key] = event_list

    def _get_event_binding_list(self, event_name):
        event_dict = self._get_event_dict()
        entry = event_dict[event_name]
        return entry[1]

    def _get_event_binding_entry_index(
            self, binding_list, target_dev_id, target_event_name):
        for i, entry in enumerate(binding_list):
            if (entry[0] == target_dev_id) and (entry[1] == target_event_name):
                return i
        raise ValueError('Binding target {} of {} not in list'.format(
            target_event_name, target_dev_id))

    def _set_event_binding_list(self, event_name, binding_list):
        event_list = self._get_event_list()
        index = self._get_event_entry_index(event_name)
        entry = event_list[index]

        entry[2] = binding_list
        event_list[index] = entry
        self._set_event_list(event_list)

    def _get_event_arg_object_type(self, type_name):
        type_map = {
            'none': None,
            'bool': bool,
            'int': int,
            'float': float,
            'tstamp': tstamp.Tstamp,
        }
        return type_map[type_name]

    def _get_event_arg_format_type(self, obj_type):
        type_map = {
            None: 'none',
            bool: 'bool',
            int: 'int',
            float: 'float',
            tstamp.Tstamp: 'tstamp',
        }
        return type_map[obj_type]

    def get_event_names(self):
        event_list = self._get_event_list()
        return [entry[0] for entry in event_list]

    def get_event_arg_types(self):
        return [None, bool, int, float, tstamp.Tstamp]

    def _get_unique_event_name(self):
        names = set('event{:02d}'.format(i) for i in range(1, 100))
        for entry in self._get_event_list():
            used_name = entry[0]
            names.discard(used_name)
        unique_name = min(names)
        return unique_name

    def add_event(self):
        event_list = self._get_event_list()
        arg_type_name = self._get_event_arg_format_type(None)
        event_name = self._get_unique_event_name()
        new_entry = [event_name, arg_type_name, []]
        event_list.append(new_entry)
        self._set_event_list(event_list)

    def remove_event(self, event_name):
        event_list = self._get_event_list()
        index = self._get_event_entry_index(event_name)
        del event_list[index]

        self._set_event_list(event_list)

    def set_event_expanded(self, event_name, expanded):
        self._session.set_au_event_expanded(self._au_id, event_name, expanded)

    def is_event_expanded(self, event_name):
        return self._session.is_au_event_expanded(self._au_id, event_name)

    def get_event_arg_type(self, event_name):
        event_dict = self._get_event_dict()
        entry = event_dict[event_name]
        return self._get_event_arg_object_type(entry[0])

    def change_event_name(self, event_name, new_name):
        event_list = self._get_event_list()
        index = self._get_event_entry_index(event_name)
        event_list[index][0] = new_name

        self._set_event_list(event_list)

    def change_event_arg_type(self, event_name, new_type):
        new_type_name = self._get_event_arg_format_type(new_type)

        event_list = self._get_event_list()
        index = self._get_event_entry_index(event_name)
        event_list[index][1] = new_type_name

        self._set_event_list(event_list)

    def get_event_binding_targets(self, event_name):
        assert event_name
        binding_list = self._get_event_binding_list(event_name)
        return [(entry[0], entry[1]) for entry in binding_list]

    def _get_unique_event_binding_event_name(self, binding_list):
        names = set('event{:02d}'.format(i) for i in range(1, 100))
        for entry in binding_list:
            used_name = entry[1]
            names.discard(used_name)
        unique_name = min(names)
        return unique_name

    def add_event_binding(self, event_name, target_dev_id, target_arg_type):
        assert event_name
        assert target_dev_id
        binding_list = self._get_event_binding_list(event_name)
        type_name = self._get_event_arg_format_type(target_arg_type)
        target_event_name = self._get_unique_event_binding_event_name(binding_list)
        expr = '' if (target_arg_type == None) else '$'
        binding_list.append([target_dev_id, target_event_name, type_name, expr])

        self._set_event_binding_list(event_name, binding_list)

    def remove_event_binding(self, event_name, target_dev_id, target_event_name):
        binding_list = self._get_event_binding_list(event_name)
        index = self._get_event_binding_entry_index(
                binding_list, target_dev_id, target_event_name)
        del binding_list[index]
        self._set_event_binding_list(event_name, binding_list)

    def get_event_binding_target_arg_type(
            self, event_name, target_dev_id, target_event_name):
        binding_list = self._get_event_binding_list(event_name)
        index = self._get_event_binding_entry_index(
                binding_list, target_dev_id, target_event_name)
        entry = binding_list[index]
        return self._get_event_arg_object_type(entry[2])

    def get_event_binding_target_expression(
            self, event_name, target_dev_id, target_event_name):
        binding_list = self._get_event_binding_list(event_name)
        index = self._get_event_binding_entry_index(
                binding_list, target_dev_id, target_event_name)
        entry = binding_list[index]
        return entry[3]

    def change_event_binding_target_dev(
            self, event_name, target_dev_id, target_event_name, new_target_dev_id):
        binding_list = self._get_event_binding_list(event_name)
        index = self._get_event_binding_entry_index(
                binding_list, target_dev_id, target_event_name)
        binding_list[index][0] = new_target_dev_id
        self._set_event_binding_list(event_name, binding_list)

    def change_event_binding_target_event_name(
            self, event_name, target_dev_id, target_event_name, new_target_event_name):
        binding_list = self._get_event_binding_list(event_name)
        index = self._get_event_binding_entry_index(
                binding_list, target_dev_id, target_event_name)
        binding_list[index][1] = new_target_event_name
        self._set_event_binding_list(event_name, binding_list)

    def change_event_binding_target_type(
            self, event_name, target_dev_id, target_event_name, new_target_arg_type):
        binding_list = self._get_event_binding_list(event_name)
        index = self._get_event_binding_entry_index(
                binding_list, target_dev_id, target_event_name)
        binding_list[index][2] = self._get_event_arg_format_type(new_target_arg_type)
        self._set_event_binding_list(event_name, binding_list)

    def change_event_binding_target_expression(
            self, event_name, target_dev_id, target_event_name, new_expr):
        binding_list = self._get_event_binding_list(event_name)
        index = self._get_event_binding_entry_index(
                binding_list, target_dev_id, target_event_name)
        binding_list[index][3] = new_expr
        self._set_event_binding_list(event_name, binding_list)


