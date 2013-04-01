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

from drivers import Drivers

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
        self._audio_drivers = Drivers()

    def set_backend(self, backend):
        pass

    def set_audio_output(self, audio_output):
        self._audio_drivers.set_audio_output(audio_output)

    def update_drivers(self, drivers):
        self._audio_drivers.update_drivers(drivers)

    def get_drivers(self):
        return self._audio_drivers


