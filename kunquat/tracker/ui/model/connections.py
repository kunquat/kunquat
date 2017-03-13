# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2017
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


