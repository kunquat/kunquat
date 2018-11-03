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
from kunquat.tracker.ui.controller.dataconverters import ConversionInfo, Converter


class ConnectionsLayoutConverterFrom0(Converter):

    def __init__(self):
        super().__init__()

    def convert_key(self, orig_key):
        return orig_key

    def convert_data(self, orig_data):
        if not isinstance(orig_data, dict):
            raise ValueError('Connections layout is not a JSON object')
        if not orig_data:
            return {}

        conv_factor = 0.09

        def is_valid_device(s):
            if not (isinstance(s, str) and (len(s) < 16)):
                return False

            if s in ('Iin', 'master'):
                return True
            if s.startswith(('au_', 'proc_')):
                parts = s.split('_')
                if (len(parts) == 2) and (len(parts[1]) == 2):
                    try:
                        value = int(parts[1], 16)
                        return (0 <= value < 256)
                    except ValueError:
                        pass
            return False

        def is_valid_coordinate_pair(v):
            if not (isinstance(v, list) and
                    (len(v) == 2) and
                    (isinstance(v[0], (int, float))) and
                    (isinstance(v[1], (int, float)))):
                return False
            return True

        new_data = {}
        for k, v in orig_data.items():
            if not isinstance(k, str):
                raise ValueError('Connections layout key is not a string')

            if k == 'z_order':
                if not isinstance(v, list):
                    raise ValueError('z_order entry is not a list of devices')
                orig_devices = v
                new_devices = []
                for i, entry in enumerate(orig_devices):
                    if not isinstance(entry, str) or not is_valid_device(entry):
                        raise ValueError('z_order list entry {} is not a device')
                    new_devices.append(entry)
                new_data['z_order'] = new_devices

            elif k == 'centre_pos':
                if not is_valid_coordinate_pair(v):
                    raise ValueError('centre_pos entry is not a coordinate pair')
                new_centre_pos = [c * conv_factor for c in v]
                new_data['centre_pos'] = new_centre_pos

            elif is_valid_device(k):
                if not isinstance(v, dict):
                    raise ValueError(
                            'Connections layout device entry is not a JSON object')

                new_desc = {}
                for desc_k, desc_v in v.items():
                    if not isinstance(desc_k, str):
                        raise ValueError(
                                'Invalid key in connections layout device entry')
                    if desc_k == 'offset':
                        if not is_valid_coordinate_pair(desc_v):
                            raise ValueError('Value in connections layout device entry'
                                    ' is not a coordinate pair')
                        new_offset = [c * conv_factor for c in desc_v]
                        new_desc['offset'] = new_offset
                    else:
                        raise ValueError(
                                'Unrecognised key in connections layout device entry')
                new_data[k] = new_desc

            else:
                raise ValueError('Unrecognised key in connections layout')

        return new_data


class Connections():

    @staticmethod
    def register_conversion_infos(data_converters):
        conns_layout_conv = ConnectionsLayoutConverterFrom0()
        info = ConversionInfo([conns_layout_conv])
        info.set_key_pattern('(au_[0-9a-f]{2}/){0,2}i_connections_layout.json')
        data_converters.add_conversion_info(info)

    def __init__(self):
        self._controller = None
        self._store = None
        self._au_id = None
        self._ui_model = None

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def _get_complete_key(self, subkey):
        parts = []
        if self._au_id:
            parts.append(self._au_id)
        parts.append(subkey)
        return '/'.join(parts)

    def _get_graph_key(self):
        return self._get_complete_key('p_connections.json')

    def _get_layout_key(self):
        return self._get_complete_key('i_connections_layout.json')

    def get_connections(self):
        key = self._get_graph_key()
        return self._store.get(key, get_default_value(key))

    def set_connections(self, conns):
        key = self._get_graph_key()
        self._store[key] = conns

    def get_layout(self):
        key = self._get_layout_key()
        return self._store.get(key, {})

    def set_layout(self, layout, mark_modified=True):
        key = self._get_layout_key()
        self._store.put({ key: layout }, mark_modified=mark_modified)

    def get_send_device_ids(self, recv_id):
        sub_recv_id = recv_id.split('/')[-1]

        send_ids = set()
        for conn in self.get_connections():
            from_path, to_path = conn
            if to_path.startswith(sub_recv_id):
                sub_send_id = from_path.split('/')[0]
                if self._au_id:
                    send_id = '/'.join((self._au_id, sub_send_id))
                else:
                    send_id = sub_send_id
                send_ids.add(send_id)

        return send_ids

    def disconnect_device(self, dev_id):
        sub_dev_id = dev_id.split('/')[-1]

        keep_conns = []
        for conn in self.get_connections():
            from_path, to_path = conn
            if not (from_path.startswith(sub_dev_id) or to_path.startswith(sub_dev_id)):
                keep_conns.append(conn)

        key = self._get_graph_key()
        self._store[key] = keep_conns

    def _get_edit_disconnect_port(self, dev_id, port_id):
        if dev_id == None:
            excluded_path = port_id
        else:
            sub_dev_id = dev_id.split('/')[-1]
            excluded_path = '{}/{}'.format(sub_dev_id, port_id)

        keep_conns = []
        for conn in self.get_connections():
            if excluded_path not in conn:
                keep_conns.append(conn)

        transaction = { self._get_graph_key(): keep_conns }
        return transaction

    def get_edit_disconnect_port(self, dev_id, port_id):
        return self._get_edit_disconnect_port(dev_id, port_id)

    def get_edit_disconnect_master_port(self, port_id):
        return self._get_edit_disconnect_port(None, port_id)


