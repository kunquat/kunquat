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

class DriverManager():

    def __init__(self):
        self._drivers = None
        self._audio_output = None
        self._selected_driver = None
        self._updater = None

    def set_drivers(self, drivers):
        self._drivers = drivers

    def set_audio_output(self, audio_output):
        self._audio_output = audio_output

    def set_updater(self, updater):
        self._updater = updater

    def get_drivers(self):
        return self._drivers

    def get_ids(self):
        if self._drivers == None:
            return []
        ids = [i.get_id() for i in self._drivers]
        return ids

    def set_selected_driver(self, driver_class):
        self._audio_output.select_driver(driver_class)

    def get_selected_driver(self):
        return self._selected_driver

    def update_selected_driver(self, driver_class):
        self._selected_driver = driver_class
        self._updater.signal_update()

