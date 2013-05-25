# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from updater import Updater

class StatManager(Updater):

    def __init__(self):
        super(StatManager, self).__init__()
        self._output_speed = 0
        self._render_speed = 0
        self._render_load = 0
        self._import_progress_position = 1
        self._import_progress_steps = 1
        self._audio_levels = (0, 0)

    def get_output_speed(self):
        return self._output_speed

    def update_output_speed(self, fps):
        self._output_speed = fps
        self._signal_update()

    def get_render_speed(self):
        return self._render_speed

    def update_render_speed(self, fps):
        self._render_speed = fps
        self._signal_update()

    def get_render_load(self):
        return self._render_load

    def update_render_load(self, ratio):
        self._render_load = ratio
        self._signal_update()

    def get_import_progress_position(self):
        return self._import_progress_position

    def get_import_progress_steps(self):
        return self._import_progress_steps

    def update_import_progress(self, position, steps):
        self._import_progress_position = position
        self._import_progress_steps = steps
        self._signal_update()

    def get_audio_levels(self):
        return self._audio_levels

    def update_audio_levels(self, levels):
        self._audio_levels = levels
        self._signal_update()

