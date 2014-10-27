# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013-2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#


class ControlManager():

    def __init__(self):
        self._session = None
        self._updater = None
        self._model = None

    def set_controller(self, controller):
        self._session = controller.get_session()
        self._updater = controller.get_updater()

    def set_model(self, model):
        self._model = model

    def get_selected_control_id(self):
        module = self._model.get_module()
        control_ids = module.get_control_ids()

        selected_id = self._session.get_control_id_override()
        if selected_id == None:
            selected_id = self._session.get_selected_control_id()

        if len(control_ids) < 1:
            return None
        if not selected_id in control_ids:
            some_id = sorted(control_ids)[0]
            return some_id
        return selected_id

    def set_selected_control_id(self, control_id):
        self._session.set_selected_control_id(control_id)
        self._updater.signal_update()

    def get_selected_control(self):
        control_id = self.get_selected_control_id()
        if control_id == None:
            return None
        module = self._model.get_module()
        control = module.get_control(control_id)
        return control

    def set_control_id_override(self, control_id):
        self._session.set_control_id_override(control_id)


