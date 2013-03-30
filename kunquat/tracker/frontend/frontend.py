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

    def __init__(self, ui_model):
        self._ui_model = ui_model

    def set_backend(self, backend):
        pass

    def set_audio_output(self, audio_output):
        self._ui_model.set_audio_output(audio_output)

    def update_drivers(self, update):
        drivers = self._ui_model.get_drivers()
        drivers.update_drivers(update)


