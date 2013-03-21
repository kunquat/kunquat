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
        self._backend = None
        self._audio_output = None

    def set_backend(self, backend):
        self._backend = backend

    def set_audio_output(self, audio_output):
        self._audio_output = audio_output

    # Ui interface

    def select_audio_driver(self, name):
        self._audio_output.select_driver(name)

