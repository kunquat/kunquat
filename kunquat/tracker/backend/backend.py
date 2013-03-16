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

class Backend():

    def __init__(self):
        self._ep = None

    def process_command(self, cmd):
        pass

    def set_audio_processor(self, ap):
        pass

    def set_event_processor(self, ep):
        """storage, environment and tracker events"""
        self._ep = ep


