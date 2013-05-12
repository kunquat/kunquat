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


class AudioOutput():

    def __init__(self):
        self._backend = None
        self._frontend = None
        self._driver = None

    def _refresh_driver_audio_source(self):
        if self._driver != None:
            self._driver.set_audio_source(self._backend)

    def select_driver(self, DriverClass):
        if self._driver:
            self._driver.close()
        if DriverClass == None:
            self._driver = None
        else:
            try:
                self._driver = DriverClass()
            except:
                self._frontend.select_driver_error(DriverClass)
                return
        if self._driver:
            self._refresh_driver_audio_source()
            self._driver.start()
        self._frontend.select_driver_success(DriverClass)
        self._backend.update_selected_driver(DriverClass)

    def set_backend(self, backend):
        self._backend = backend
        self._refresh_driver_audio_source()

    def set_frontend(self, frontend):
        self._frontend = frontend

    def put_audio(self, audio):
        if self._driver:
            self._driver.put_audio(audio)
            

