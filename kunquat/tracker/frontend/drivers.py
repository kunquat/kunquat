# -*- coding: utf-8 -*-

#
# Author: Toni Ruottu, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

class Drivers():

    def __init__(self):
        self._drivers = None
        self._audio_output = None

    def set_audio_output(self, audio_output):
        self._audio_output = audio_output

    def update_drivers(self, drivers):
        self._drivers = drivers

    def get_ids(self):
        if self._drivers == None:
            return []
        return self._drivers.keys()

    def select_audio_driver(self, name):
        self._audio_output.select_driver(name)

