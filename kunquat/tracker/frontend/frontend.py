# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013
#          Toni Ruottu, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#


class Frontend():

    def __init__(self):
        self._ui_model = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    # Frontend Interface

    def set_backend(self, backend):
        self._ui_model.set_backend(backend)

    def set_audio_output(self, audio_output):
        self._ui_model.set_audio_output(audio_output)

    def update_import_progress(self, position, steps):
        stats = self._ui_model.get_stat_manager()
        stats.update_import_progress(position, steps)

    def update_output_speed(self, fps):
        stats = self._ui_model.get_stat_manager()
        stats.update_output_speed(fps)

    def update_render_speed(self, fps):
        stats = self._ui_model.get_stat_manager()
        stats.update_render_speed(fps)

    def update_render_load(self, ratio):
        stats = self._ui_model.get_stat_manager()
        stats.update_render_load(ratio)

    def update_drivers(self, update):
        drivers = self._ui_model.get_drivers()
        drivers.update_drivers(update)

    def update_selected_driver(self, driver_class):
        driver_manager = self._ui_model.get_driver_manager()
        driver_manager.update_selected_driver(driver_class)

    def update_audio_levels(self, levels):
        stats = self._ui_model.get_stat_manager()
        stats.update_audio_levels(levels)

