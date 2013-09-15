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
        self._backend = None
        self._InstrumentClass = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def set_instrument_class(self, InstrumentClass):
        self._InstrumentClass = InstrumentClass

    # Frontend Interface

    def set_backend(self, backend):
        self._backend = backend
        self._ui_model.set_backend(backend)

    def set_audio_output(self, audio_output):
        self._ui_model.set_audio_output(audio_output)

    def _create_and_get_instrument(self, instrument_number):
        module = self._ui_model.get_module()
        instruments = module.get_instruments(validate=False)
        instrument_numbers = [i.get_instrument_number() for i in instruments]
        if not instrument_number in instrument_numbers:
            new_instrument = self._InstrumentClass()
            new_instrument.set_backend(self._backend)
            new_instrument.set_instrument_number(instrument_number)
            module.update_instrument(instrument_number, new_instrument)
        instrument = module.get_instrument(instrument_number)
        return instrument

    def update_selected_instrument(self, channel_number, instrument_number):
        playback_manager = self._ui_model.get_playback_manager()
        channel = playback_manager.get_channel(channel_number)
        channel.update_selected_instrument_number(instrument_number)

    def update_instrument_existence(self, instrument_number, existence):
        instrument = self._create_and_get_instrument(instrument_number)
        instrument.update_existence(existence)

    def update_instrument_name(self, instrument_number, name):
        instrument = self._create_and_get_instrument(instrument_number)
        instrument.update_name(name)

    def update_active_note(self, channel_number, pitch):
        module = self._ui_model.get_module()
        playback_manager = self._ui_model.get_playback_manager()
        channel = playback_manager.get_channel(channel_number)
        active_instrument_number = channel.get_active_instrument_number()
        if active_instrument_number != None:
            active_instrument = module.get_instrument(active_instrument_number)
            active_instrument.update_active_note(channel_number, None)
        selected_instrument_number = channel.get_selected_instrument_number()
        if selected_instrument_number != None:
            selected_instrument = module.get_instrument(selected_instrument_number)
            selected_instrument.update_active_note(channel_number, pitch)

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

