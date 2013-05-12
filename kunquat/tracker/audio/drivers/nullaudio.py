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

class Nullaudio():

    def __init__(self):
        self._audio_source = None

    def set_audio_source(self, audio_source):
        self._audio_source = audio_source

    def put_audio(self, audio_data):
        self._audio_source.acknowledge_audio()

    def start(self):
        pass

    def stop(self):
        pass

    def close(self):
        self.stop()

    @classmethod
    def get_id(cls):
        return 'nullaudio'

    #'Silent null driver'

