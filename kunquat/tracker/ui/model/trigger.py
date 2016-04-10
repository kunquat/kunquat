# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import kunquat.kunquat.events as events
from .control import Control


class InvArg():
    pass


class Trigger():

    def __init__(self, trigger_type, argument=InvArg, location=None):
        if isinstance(trigger_type, Trigger):
            tr = trigger_type
            trigger_type = tr._type
            argument = tr._argument
            location = tr._location
        else:
            assert argument != InvArg

        assert (argument == None) or (type(argument) == str)
        self._type = trigger_type
        self._argument = argument
        self._location = location
        self._ui_model = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def get_type(self):
        return self._type

    def get_argument(self):
        return self._argument

    def get_argument_type(self):
        try:
            return events.trigger_events_by_name[self._type]['arg_type']
        except KeyError:
            return None

    def get_hit_name(self):
        assert self._type == 'h'
        assert self._location

        name = str(self._argument)
        try:
            hit_index = int(self._argument)
        except ValueError:
            return name

        sheet_manager = self._ui_model.get_sheet_manager()
        control_id = sheet_manager.get_inferred_active_control_id_at_location(
                self._location)
        module = self._ui_model.get_module()
        control = module.get_control(control_id)
        if control.get_existence():
            au = control.get_audio_unit()
            if au.get_existence():
                hit = au.get_hit(hit_index)
                if hit.get_existence():
                    hit_name = hit.get_name()
                    if hit_name:
                        name = hit_name

        return name


