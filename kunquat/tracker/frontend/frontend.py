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

class Frontend():

    def __init__(self):
        self._audio_drivers = Drivers()

    def set_backend(self, backend):
        pass

    def set_audio_output(self, audio_output):
        self._audio_drivers.set_audio_output(audio_output)

    # Audio interface

    def update_drivers(self, drivers):
        self._audio_drivers.update_drivers(drivers)

    # Ui interface

    def get_drivers(self):
        return self._audio_drivers

