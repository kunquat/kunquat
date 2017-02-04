# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import types
from collections import OrderedDict


class Updater():

    def __init__(self):
        self._ui_model = None
        self._updater = None

        self._signal_action_map = OrderedDict()
        self._updating_children = []

    def __getattr__(self, name):
        # This hack is needed because PySide widgets do not cooperate with
        # multiple inheritance, hence our __init__ might not get called
        if name in ('_ui_model', '_updater'):
            return None
        elif name == '_signal_action_map':
            self._signal_action_map = OrderedDict()
            return self._signal_action_map
        elif name == '_updating_children':
            self._updating_children = []
            return self._updating_children

        raise AttributeError('Unexpected field access: {}'.format(name))

    def add_to_updaters(self, *widgets):
        for widget in widgets:
            assert widget not in self._updating_children
        self._updating_children.extend(widgets)
        if self._updater:
            for widget in widgets:
                widget.set_ui_model(self._ui_model)

    def remove_from_updaters(self, widget):
        self._updating_children.remove(widget)
        if self._updater:
            widget.unregister_updaters()

    def register_action(self, signal, action):
        assert signal not in self._signal_action_map

        if type(action) == types.BuiltinMethodType:
            action_id = (id(action.__self__), id(action))
        elif type(action) == types.MethodType:
            action_id = (id(action.__self__), id(action.__func__))
        else:
            assert False, 'Unexpected action type: {}'.format(type(action))

        action_info = (action, action_id)
        self._signal_action_map[signal] = action_info
        if self._updater:
            self._updater.register_updater(self._update)

    def set_ui_model(self, ui_model):
        assert not self._ui_model
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        if self._signal_action_map:
            self._updater.register_updater(self._update)

        for view in self._updating_children:
            view.set_ui_model(ui_model)

        self._on_setup()

    def unregister_updaters(self):
        self._on_teardown()

        for view in reversed(self._updating_children):
            view.unregister_updaters()

        if self._signal_action_map:
            self._updater.unregister_updater(self._update)

    def _update(self, signals):
        called_action_ids = set()
        for signal, action_info in self._signal_action_map.items():
            action, action_id = action_info
            if (signal in signals) and (action_id not in called_action_ids):
                called_action_ids.add(action_id)
                action()

    # Protected callbacks

    def _on_setup(self):
        pass

    def _on_teardown(self):
        pass


