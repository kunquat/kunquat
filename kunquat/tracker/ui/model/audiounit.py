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

from kunquat.kunquat.kunquat import get_default_value
from kunquat.kunquat.limits import *
from connections import Connections
from processor import Processor
import tstamp


# Default processor settings
_proc_defaults = {
    'add':      { 'signal_type': u'voice',
                  'ports': ['in_00', 'in_01', 'in_02', 'out_00', 'out_01'] },
    'chorus':   { 'signal_type': u'mixed',
                  'ports': ['in_00', 'in_01', 'out_00', 'out_01'] },
    'delay':    { 'signal_type': u'mixed',
                  'ports': ['in_00', 'in_01', 'out_00', 'out_01'] },
    'envgen':   { 'signal_type': u'voice', 'ports': ['out_00'] },
    'filter':   { 'signal_type': u'mixed',
                  'ports': ['in_00', 'in_01', 'in_02', 'out_00', 'out_01'] },
    'force':    { 'signal_type': u'voice', 'ports': ['out_00'] },
    'freeverb': { 'signal_type': u'mixed',
                  'ports': ['in_00', 'in_01', 'out_00', 'out_01'] },
    'gaincomp': { 'signal_type': u'mixed',
                  'ports': ['in_00', 'in_01', 'out_00', 'out_01'] },
    'panning':  { 'signal_type': u'mixed',
                  'ports': ['in_00', 'in_01', 'out_00', 'out_01'] },
    'pitch':    { 'signal_type': u'voice', 'ports': ['out_00'] },
    'ringmod':  { 'signal_type': u'mixed',
                  'ports': ['in_00', 'in_01', 'in_02', 'in_03', 'out_00', 'out_01'] },
    'sample':   { 'signal_type': u'voice', 'ports': ['out_00', 'out_01'] },
    'volume':   { 'signal_type': u'mixed',
                  'ports': ['in_00', 'in_01', 'in_02', 'out_00', 'out_01'] },
}


FLOAT_SLIDE_TYPE = 'float_slide'


class AudioUnit():

    def __init__(self, au_id):
        assert au_id
        assert len(au_id.split('/')) <= 2
        self._au_id = au_id
        self._store = None
        self._controller = None
        self._session = None
        self._ui_model = None
        self._au_number = None
        self._existence = None

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller
        self._session = controller.get_session()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def get_id(self):
        return self._au_id

    def _get_key(self, subkey):
        return '{}/{}'.format(self._au_id, subkey)

    def set_existence(self, au_type):
        assert (not au_type) or (au_type in ('instrument', 'effect'))
        key = self._get_key('p_manifest.json')
        if au_type:
            manifest = { 'type': au_type }
            self._store[key] = manifest
        else:
            del self._store[key]

    def get_existence(self):
        key = self._get_key('p_manifest.json')
        manifest = self._store.get(key, None)
        return (type(manifest) == dict)

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
        for i in xrange(0x100):
            port_id = 'in_{:02x}'.format(i)
            key = self._get_key('{}/p_manifest.json'.format(port_id))
            if key in self._store:
                in_ports.append(port_id)

        return in_ports

    def get_out_ports(self):
        out_ports = []
        for i in xrange(0x100):
            port_id = 'out_{:02x}'.format(i)
            key = self._get_key('{}/p_manifest.json'.format(port_id))
            if key in self._store:
                out_ports.append(port_id)

        return out_ports

    def set_port_existence(self, port_id, existence):
        key = self._get_key('{}/p_manifest.json'.format(port_id))
        if existence:
            self._store[key] = {}
        else:
            del self._store[key]

    def get_connections(self):
        connections = Connections()
        connections.set_au_id(self._au_id)
        connections.set_controller(self._controller)
        connections.set_ui_model(self._ui_model)
        return connections

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
                for i in xrange(PROCESSORS_MAX))
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

        signal_type_key = '{}/p_signal_type.json'.format(key_prefix)
        transaction[signal_type_key] = _proc_defaults[proc_type]['signal_type']

        for port_id in _proc_defaults[proc_type]['ports']:
            port_manifest_key = '{}/{}/p_manifest.json'.format(key_prefix, port_id)
            transaction[port_manifest_key] = {}

        self._store.put(transaction)

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
                for i in xrange(AUDIO_UNITS_MAX))
        used_au_ids = self.get_au_ids()
        free_au_ids = all_au_ids - used_au_ids
        if not free_au_ids:
            return None
        return min(free_au_ids)

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

    def _remove_device(self, dev_id):
        assert dev_id.startswith(self._au_id)

        transaction = {}
        start = dev_id + '/'
        for key in self._store.iterkeys():
            if key.startswith(start):
                transaction[key] = None

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
        assert isinstance(name, unicode)
        key = self._get_key('m_name.json')
        self._store[key] = name

    def _get_control_var_list(self):
        key = self._get_key('p_control_vars.json')
        ret = self._store.get(key, get_default_value(key))
        return ret

    def _get_control_var_dict(self):
        var_list = self._get_control_var_list()
        var_dict = {}
        for entry in var_list:
            type_name, var_name, init_value, ext, bindings = entry
            var_dict[var_name] = (type_name, init_value, ext, bindings)
        return var_dict

    def _get_control_var_entry_index(self, var_list, var_name):
        for i, entry in enumerate(var_list):
            if entry[1] == var_name:
                return i
        raise ValueError('Variable name {} not in list'.format(var_name))

    def _set_control_var_list(self, var_list):
        key = self._get_key('p_control_vars.json')
        self._store[key] = var_list

    def _get_control_var_binding_list(self, var_name):
        var_dict = self._get_control_var_dict()
        var_entry = var_dict[var_name]
        return var_entry[3]

    def _get_control_var_binding_entry_index(
            self, binding_list, target_dev_id, target_var_name):
        for i, entry in enumerate(binding_list):
            if (entry[0] == target_dev_id) and (entry[1] == target_var_name):
                return i
        raise ValueError('Binding target {} not in list'.format(target_var_name))

    def _set_control_var_binding_list(self, var_name, binding_list):
        var_list = self._get_control_var_list()
        index = self._get_control_var_entry_index(var_list, var_name)
        var_entry = var_list[index]

        var_entry[4] = binding_list
        var_list[index] = var_entry
        self._set_control_var_list(var_list)

    def _get_control_var_object_type(self, type_name):
        type_map = {
                'bool': bool,
                'int': int,
                'float': float,
                'tstamp': tstamp.Tstamp,
                FLOAT_SLIDE_TYPE: FLOAT_SLIDE_TYPE,
            }
        return type_map[type_name]

    def _get_control_var_format_type(self, obj_type):
        type_map = {
                bool: 'bool',
                int: 'int',
                float: 'float',
                tstamp.Tstamp: 'tstamp',
                FLOAT_SLIDE_TYPE: FLOAT_SLIDE_TYPE,
            }
        return type_map[obj_type]

    def get_control_var_names(self):
        var_list = self._get_control_var_list()
        return [var[1] for var in var_list]

    def get_control_var_types(self):
        return [bool, int, float, tstamp.Tstamp, FLOAT_SLIDE_TYPE]

    def get_control_var_binding_target_types(self):
        return [bool, int, float, tstamp.Tstamp]

    def _get_unique_control_var_name(self, var_list):
        names = set('var{:02d}'.format(i) for i in xrange(1, 100))
        for entry in var_list:
            used_name = entry[1]
            names.discard(used_name)
        unique_name = min(names)
        return unique_name

    def add_control_var_float_slide(self):
        var_list = self._get_control_var_list()
        type_name = self._get_control_var_format_type(FLOAT_SLIDE_TYPE)
        var_name = self._get_unique_control_var_name(var_list)
        new_entry = [type_name, var_name, 0.0, [0.0, 1.0], []]
        var_list.append(new_entry)
        self._set_control_var_list(var_list)

    def remove_control_var(self, var_name):
        var_list = self._get_control_var_list()
        index = self._get_control_var_entry_index(var_list, var_name)
        del var_list[index]

        self._set_control_var_list(var_list)

    def set_control_var_expanded(self, var_name, expanded):
        self._session.set_au_var_expanded(self._au_id, var_name, expanded)

    def is_control_var_expanded(self, var_name):
        return self._session.is_au_var_expanded(self._au_id, var_name)

    def get_control_var_type(self, var_name):
        var_dict = self._get_control_var_dict()
        var_entry = var_dict[var_name]
        return self._get_control_var_object_type(var_entry[0])

    def get_control_var_init_value(self, var_name):
        var_dict = self._get_control_var_dict()
        var_entry = var_dict[var_name]
        var_type = self._get_control_var_object_type(var_entry[0])
        if var_type == FLOAT_SLIDE_TYPE:
            var_type = float
        return var_type(var_entry[1])

    def get_control_var_min_value(self, var_name):
        var_dict = self._get_control_var_dict()
        var_entry = var_dict[var_name]
        assert self._get_control_var_object_type(var_entry[0]) == FLOAT_SLIDE_TYPE
        ext = var_entry[2]
        return ext[0]

    def get_control_var_max_value(self, var_name):
        var_dict = self._get_control_var_dict()
        var_entry = var_dict[var_name]
        assert self._get_control_var_object_type(var_entry[0]) == FLOAT_SLIDE_TYPE
        ext = var_entry[2]
        return ext[1]

    def change_control_var_name(self, var_name, new_name):
        var_list = self._get_control_var_list()
        index = self._get_control_var_entry_index(var_list, var_name)
        var_list[index][1] = new_name

        self._set_control_var_list(var_list)

    def change_control_var_type(self, var_name, new_type):
        new_type_name = self._get_control_var_format_type(new_type)

        cons = new_type if new_type != FLOAT_SLIDE_TYPE else float

        var_list = self._get_control_var_list()
        index = self._get_control_var_entry_index(var_list, var_name)
        var_list[index][0] = new_type_name
        var_list[index][2] = cons(0)

        # Set default extended params as they are type-specific
        if new_type in (bool, int, float, tstamp.Tstamp):
            var_list[index][3] = []
        elif new_type == FLOAT_SLIDE_TYPE:
            var_list[index][3] = [0.0, 1.0]
        else:
            raise NotImplementedError

        # Remove existing bindings as they are type-specific
        var_list[index][4] = []

        self._set_control_var_list(var_list)

    def change_control_var_init_value(self, var_name, new_value):
        var_list = self._get_control_var_list()
        index = self._get_control_var_entry_index(var_list, var_name)
        var_list[index][2] = new_value

        var_type = self._get_control_var_object_type(var_list[index][0])
        if var_type == FLOAT_SLIDE_TYPE:
            ext = var_list[index][3]
            var_list[index][2] = min(max(ext[0], new_value), ext[1])

        self._set_control_var_list(var_list)

    def change_control_var_min_value(self, var_name, new_value):
        var_list = self._get_control_var_list()
        index = self._get_control_var_entry_index(var_list, var_name)
        assert self._get_control_var_object_type(var_list[index][0]) == FLOAT_SLIDE_TYPE
        ext = var_list[index][3]
        ext[0] = new_value

        var_list[index][2] = max(var_list[index][2], new_value)
        ext[1] = max(ext[1], new_value)

        self._set_control_var_list(var_list)

    def change_control_var_max_value(self, var_name, new_value):
        var_list = self._get_control_var_list()
        index = self._get_control_var_entry_index(var_list, var_name)
        assert self._get_control_var_object_type(var_list[index][0]) == FLOAT_SLIDE_TYPE
        ext = var_list[index][3]
        ext[1] = new_value

        var_list[index][2] = min(var_list[index][2], new_value)
        ext[0] = min(ext[0], new_value)

        self._set_control_var_list(var_list)

    def get_control_var_binding_targets(self, var_name):
        assert var_name
        binding_list = self._get_control_var_binding_list(var_name)
        return [(entry[0], entry[1]) for entry in binding_list]

    def _get_unique_binding_var_name(self, binding_list):
        names = set('var{:02d}'.format(i) for i in xrange(1, 100))
        for entry in binding_list:
            used_name = entry[1]
            names.discard(used_name)
        unique_name = min(names)
        return unique_name

    def add_control_var_binding(
            self, var_name, target_dev_id, target_var_type):
        assert var_name
        assert target_dev_id
        binding_list = self._get_control_var_binding_list(var_name)
        type_name = self._get_control_var_format_type(target_var_type)
        target_var_name = self._get_unique_binding_var_name(binding_list)
        binding_list.append([
            target_dev_id, target_var_name, type_name, '$'])
        self._set_control_var_binding_list(var_name, binding_list)

    def add_control_var_binding_float_slide(
            self, var_name, target_dev_id, map_min_to, map_max_to):
        assert var_name
        assert target_dev_id
        binding_list = self._get_control_var_binding_list(var_name)
        type_name = self._get_control_var_format_type(float)
        target_var_name = self._get_unique_binding_var_name(binding_list)
        binding_list.append([
            target_dev_id, target_var_name, type_name, map_min_to, map_max_to])
        self._set_control_var_binding_list(var_name, binding_list)

    def remove_control_var_binding(self, var_name, target_dev_id, target_var_name):
        binding_list = self._get_control_var_binding_list(var_name)
        index = self._get_control_var_binding_entry_index(
                binding_list, target_dev_id, target_var_name)
        del binding_list[index]
        self._set_control_var_binding_list(var_name, binding_list)

    def get_control_var_binding_target_type(
            self, var_name, target_dev_id, target_var_name):
        binding_list = self._get_control_var_binding_list(var_name)
        index = self._get_control_var_binding_entry_index(
                binding_list, target_dev_id, target_var_name)
        entry = binding_list[index]
        return self._get_control_var_object_type(entry[2])

    def get_control_var_binding_expression(
            self, var_name, target_dev_id, target_var_name):
        binding_list = self._get_control_var_binding_list(var_name)
        index = self._get_control_var_binding_entry_index(
                binding_list, target_dev_id, target_var_name)
        entry = binding_list[index]
        return entry[3]

    def get_control_var_binding_map_to_min(
            self, var_name, target_dev_id, target_var_name):
        binding_list = self._get_control_var_binding_list(var_name)
        index = self._get_control_var_binding_entry_index(
                binding_list, target_dev_id, target_var_name)
        entry = binding_list[index]
        return entry[3]

    def get_control_var_binding_map_to_max(
            self, var_name, target_dev_id, target_var_name):
        binding_list = self._get_control_var_binding_list(var_name)
        index = self._get_control_var_binding_entry_index(
                binding_list, target_dev_id, target_var_name)
        entry = binding_list[index]
        return entry[4]

    def change_control_var_binding_target_dev(
            self, var_name, target_dev_id, target_var_name, new_target_dev_id):
        binding_list = self._get_control_var_binding_list(var_name)
        index = self._get_control_var_binding_entry_index(
                binding_list, target_dev_id, target_var_name)
        binding_list[index][0] = new_target_dev_id
        self._set_control_var_binding_list(var_name, binding_list)

    def change_control_var_binding_target_name(
            self, var_name, target_dev_id, target_var_name, new_target_name):
        binding_list = self._get_control_var_binding_list(var_name)
        index = self._get_control_var_binding_entry_index(
                binding_list, target_dev_id, target_var_name)
        binding_list[index][1] = new_target_name
        self._set_control_var_binding_list(var_name, binding_list)

    def change_control_var_binding_target_type(
            self, var_name, target_dev_id, target_var_name, new_type):
        new_type_name = self._get_control_var_format_type(new_type)

        binding_list = self._get_control_var_binding_list(var_name)
        index = self._get_control_var_binding_entry_index(
                binding_list, target_dev_id, target_var_name)
        binding_list[index][2] = new_type_name
        self._set_control_var_binding_list(var_name, binding_list)

    def change_control_var_binding_expression(
            self, var_name, target_dev_id, target_var_name, expr):
        binding_list = self._get_control_var_binding_list(var_name)
        index = self._get_control_var_binding_entry_index(
                binding_list, target_dev_id, target_var_name)
        entry = binding_list[index]
        entry[3] = expr
        self._set_control_var_binding_list(var_name, binding_list)

    def change_control_var_binding_map_to_min(
            self, var_name, target_dev_id, target_var_name, new_value):
        binding_list = self._get_control_var_binding_list(var_name)
        index = self._get_control_var_binding_entry_index(
                binding_list, target_dev_id, target_var_name)
        entry = binding_list[index]
        entry[3] = new_value
        self._set_control_var_binding_list(var_name, binding_list)

    def change_control_var_binding_map_to_max(
            self, var_name, target_dev_id, target_var_name, new_value):
        binding_list = self._get_control_var_binding_list(var_name)
        index = self._get_control_var_binding_entry_index(
                binding_list, target_dev_id, target_var_name)
        entry = binding_list[index]
        entry[4] = new_value
        self._set_control_var_binding_list(var_name, binding_list)


