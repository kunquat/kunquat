# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2015
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.kunquat.kunquat import get_default_value


class Connections():

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

    def set_layout(self, layout):
        key = self._get_layout_key()
        self._store[key] = layout

    def is_proc_out_connected_to_mixed_device(self, proc_id):
        assert self._au_id
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        sub_proc_id = proc_id.split('/')[-1]
        for conn in self.get_connections():
            from_path, to_path = conn

            is_target_mixed = True
            if not to_path.startswith('out_'):
                target_sub_id = to_path.split('/')[0]
                if target_sub_id.startswith('proc_'):
                    target_id = '/'.join((self._au_id, target_sub_id))
                    proc = au.get_processor(target_id)
                    is_target_mixed = (proc.get_signal_type() == 'mixed')

            if is_target_mixed and from_path.split('/')[0] == sub_proc_id:
                return True

        return False

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


