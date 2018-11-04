# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2017-2018
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
        super().__init__()
        self._ui_model = None
        self._updater = None

        self._updater_setup_done = False
        self._actions_to_be_sent = []
        self._updating_children = []

    def __getattr__(self, name):
        # This hack is needed because PySide widgets do not cooperate with
        # multiple inheritance, hence our __init__ might not get called
        if name in ('_ui_model', '_updater'):
            return None
        elif name == '_actions_to_be_sent':
            self._actions_to_be_sent = []
            return self._actions_to_be_sent
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

    def _add_registered_actions(self):
        self._updater.register_actions(self._actions_to_be_sent)
        self._actions_to_be_sent = []

    def register_action(self, signal, action):
        if type(action) == types.BuiltinMethodType:
            action_id = (id(action.__self__), id(action))
        elif type(action) == types.MethodType:
            action_id = (id(action.__self__), id(action.__func__))
        else:
            assert False, 'Unexpected action type: {}'.format(type(action))

        action_info = (id(self), action, action_id)
        self._actions_to_be_sent.append((signal, action_info))

        if self._updater_setup_done:
            self._add_registered_actions()

    def set_ui_model(self, ui_model):
        assert not self._ui_model
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        for view in self._updating_children:
            view.set_ui_model(ui_model)

        self._on_setup()
        self._updater_setup_done = True

        self._add_registered_actions()

    def unregister_updaters(self):
        self._on_teardown()

        for view in reversed(self._updating_children):
            view.unregister_updaters()

        self._updater.unregister_actions(id(self))

    # Protected callbacks

    def _on_setup(self):
        pass

    def _on_teardown(self):
        pass


