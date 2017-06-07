# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#


class StatManager():

    def __init__(self):
        self._output_speed = 0
        self._render_speed = 0
        self._render_load = 0
        self._audio_levels = (0, 0)
        self._updater = None
        self._session = None

    def set_controller(self, controller):
        self._updater = controller.get_updater()
        self._session = controller.get_session()

    def get_output_speed(self):
        return self._session.get_output_speed()

    def get_render_speed(self):
        return self._session.get_render_speed()

    def get_render_load(self):
        return self._session.get_render_load()

    def get_render_load_averages(self):
        return self._session.get_render_load_averages()

    def get_render_load_peaks(self):
        return self._session.get_render_load_peaks()

    def get_ui_load(self):
        return self._session.get_ui_load()

    def get_ui_load_averages(self):
        return self._session.get_ui_load_averages()

    def get_ui_load_peaks(self):
        return self._session.get_ui_load_peaks()

    def get_progress_description(self):
        return self._session.get_progress_description()

    def get_progress_norm(self):
        return self._session.get_progress_position()

    def get_import_progress_position(self):
        return self._session.get_progress_position()

    def get_audio_levels(self):
        return self._session.get_audio_levels()

    def get_max_audio_levels(self):
        return self._session.get_max_audio_levels()


