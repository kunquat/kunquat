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

from driver_manager import DriverManager
from stat_manager import StatManager
from ui_manager import UiManager
from playback_manager import PlaybackManager
from module import Module

class UiModel():
    """
    >>> ui_model = UiModel()
    >>> project = ui_model.get_project()
    >>> module = project.get_module()
    >>> module.instruments()
    []
    >>> module.create_instrument()
    >>> module.instruments()
    [0]
    >>> instrument = module.get_instrument(0)
    >>> instrument.generators()
    []
    >>> instrument.create_generator()
    >>> instrument.generators()
    [0]
    >>> generator = instrument.get_generator(0)
    >>> generator.set_type('debug')
    >>> instrument_output = instrument.get_output(0)
    >>> genrator_output = generator.get_output(0)
    >>> instrument.list_connections()
    []
    >>> instrument.connect(generator_output, instrument_master)
    >>> instrument.list_connections()
    [(<generator_output>, <instrument_output>)]
    >>> master_output = module.get_output()
    >>> module.connect(instrument_output, master_output)
    >>> module.track_count()
    0
    >>> module.create_track()
    >>> module.track_count()
    1
    >>> song = module.get_song_by_track(0)
    >>> song.system_count()
    1
    >>> pattern_instance = song.get_pattern_instance_by_system(0)
    >>> pattern = pattern_instance.get_pattern()

    Insert Notes
    >>> pattern.

    
    >>> song.insert_new_instance(pattern)
    >>> system_count = song.system_count()
    >>> new_instance_number = system_count - 1
    >>> new_instance = song.get_pattern_instance_by_system(new_instance_number)
    >>> target_system_number = 0
    >>> song.move_here(target_system_number, new_instance)


    >>> song.pattern_instance_count()
    0
    >>> song.create_pattern_instance()
    >>> song.pattern_instance_count()
    1
   

    """

    def __init__(self):
        self._driver_manager = None
        self._stat_manager = None
        self._ui_manager = None
        self._playback_manager = None
        self._module = None
        self._updater = None
        self._store = None

    def set_backend(self, backend):
        self._backend = backend
        self._module.set_backend(self._backend)

    def set_ui(self, ui):
        self._ui = ui

    def set_updater(self, updater):
        self._updater = updater
        self._stat_manager.set_updater(self._updater)
        self._ui_manager.set_updater(self._updater)
        self._module.set_updater(self._updater)

    def set_store(self, store):
        self._store = store
        self._module.set_store(store)

    def get_updater(self):
        return self._updater

    def set_driver_manager(self, driver_manager):
        self._driver_manager = driver_manager

    def get_driver_manager(self):
        return self._driver_manager

    def set_stat_manager(self, stat_manager):
        self._stat_manager = stat_manager

    def get_stat_manager(self):
        return self._stat_manager

    def set_ui_manager(self, ui_manager):
        self._ui_manager = ui_manager

    def get_ui_manager(self):
        return self._ui_manager

    def set_playback_manager(self, playback_manager):
        self._playback_manager = playback_manager

    def get_playback_manager(self):
        return self._playback_manager

    def set_module(self, module):
        self._module = module

    def get_module(self):
        return self._module

    def set_audio_output(self, audio_output):
        self._driver_manager.set_audio_output(audio_output)

    def load_module(self):
        self._backend.load_module()

    def play(self):
        self._backend.play()


def create_ui_model():
    stat_manager = StatManager()
    ui_manager = UiManager()
    playback_manager = PlaybackManager()
    module = Module()
    ui_model = UiModel()
    ui_model.set_stat_manager(stat_manager)
    ui_model.set_ui_manager(ui_manager)
    ui_model.set_playback_manager(playback_manager)
    ui_model.set_module(module)
    #ui_model.set_backend(self._backend)
    return ui_model

